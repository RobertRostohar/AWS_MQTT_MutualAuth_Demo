#include "FreeRTOS.h"
#include "semphr.h"

#include "core_pkcs11.h"
#include "iot_crypto.h"

/**
 * @brief Create a static PKCS #11 crypto session handle to share across socket
 * and FreeRTOS+TCP threads. Assume that two or more threads may race to be the
 * first to initialize the static and handle that case accordingly.
 */
static CK_RV prvSocketsGetCryptoSession( SemaphoreHandle_t * pxSessionLock,
                                         CK_SESSION_HANDLE * pxSession,
                                         CK_FUNCTION_LIST_PTR_PTR ppxFunctionList )
{
    CK_RV xResult = CKR_OK;
    CK_C_GetFunctionList pxCkGetFunctionList = NULL;
    static CK_SESSION_HANDLE xPkcs11Session = 0;
    static CK_FUNCTION_LIST_PTR pxPkcs11FunctionList = NULL;
    static StaticSemaphore_t xStaticSemaphore;
    static SemaphoreHandle_t xSessionLock = NULL;
    CK_ULONG ulCount = 0;
    CK_SLOT_ID * pxSlotIds = NULL;

    /* Check if one-time initialization of the lock is needed.*/
    portENTER_CRITICAL();

    if( NULL == xSessionLock )
    {
        xSessionLock = xSemaphoreCreateMutexStatic( &xStaticSemaphore );
    }

    *pxSessionLock = xSessionLock;
    portEXIT_CRITICAL();

    /* Check if one-time initialization of the crypto handle is needed.*/
    xSemaphoreTake( xSessionLock, portMAX_DELAY );

    if( 0 == xPkcs11Session )
    {
        /* One-time initialization. */

        /* Ensure that the PKCS#11 module is initialized. We don't keep the
         * scheduler stopped here, since we don't want to make assumptions about hardware
         * requirements for accessing a crypto module. */

        pxCkGetFunctionList = C_GetFunctionList;
        xResult = pxCkGetFunctionList( &pxPkcs11FunctionList );

        if( CKR_OK == xResult )
        {
            xResult = xInitializePkcs11Token();
        }

        /* Get the crypto token slot count. */
        if( CKR_OK == xResult )
        {
            xResult = pxPkcs11FunctionList->C_GetSlotList( CK_TRUE,
                                                           NULL,
                                                           &ulCount );
        }

        /* Allocate memory to store the token slots. */
        if( CKR_OK == xResult )
        {
            pxSlotIds = ( CK_SLOT_ID * ) pvPortMalloc( sizeof( CK_SLOT_ID ) * ulCount );

            if( NULL == pxSlotIds )
            {
                xResult = CKR_HOST_MEMORY;
            }
        }

        /* Get all of the available private key slot identities. */
        if( CKR_OK == xResult )
        {
            xResult = pxPkcs11FunctionList->C_GetSlotList( CK_TRUE,
                                                           pxSlotIds,
                                                           &ulCount );
        }

        /* Start a session with the PKCS#11 module. */
        if( CKR_OK == xResult )
        {
            xResult = pxPkcs11FunctionList->C_OpenSession( pxSlotIds[ 0 ],
                                                           CKF_SERIAL_SESSION,
                                                           NULL,
                                                           NULL,
                                                           &xPkcs11Session );
        }
    }

    *pxSession = xPkcs11Session;
    *ppxFunctionList = pxPkcs11FunctionList;
    xSemaphoreGive( xSessionLock );

    if( NULL != pxSlotIds )
    {
        vPortFree( pxSlotIds );
    }

    return xResult;
}

/**
 * @brief Generate a TCP Initial Sequence Number that is reasonably difficult
 * to predict, per https://tools.ietf.org/html/rfc6528.
 */
uint32_t ulApplicationGetNextSequenceNumber( uint32_t ulSourceAddress,
                                             uint16_t usSourcePort,
                                             uint32_t ulDestinationAddress,
                                             uint16_t usDestinationPort )
{
    CK_RV xResult = CKR_OK;
    SemaphoreHandle_t xSessionLock = NULL;
    CK_SESSION_HANDLE xPkcs11Session = 0;
    CK_FUNCTION_LIST_PTR pxPkcs11FunctionList = NULL;
    CK_MECHANISM xMechSha256 = { 0 };
    uint8_t ucSha256Result[ cryptoSHA256_DIGEST_BYTES ];
    CK_ULONG ulLength = sizeof( ucSha256Result );
    uint32_t ulNextSequenceNumber = 0;
    static uint64_t ullKey;
    static CK_BBOOL xKeyIsInitialized = CK_FALSE;

    /* Acquire a crypto session handle. */
    xResult = prvSocketsGetCryptoSession( &xSessionLock,
                                          &xPkcs11Session,
                                          &pxPkcs11FunctionList );

    if( CKR_OK == xResult )
    {
        xSemaphoreTake( xSessionLock, portMAX_DELAY );

        if( CK_FALSE == xKeyIsInitialized )
        {
            /* One-time initialization, per boot, of the random seed. */
            xResult = pxPkcs11FunctionList->C_GenerateRandom( xPkcs11Session,
                                                              ( CK_BYTE_PTR ) &ullKey,
                                                              sizeof( ullKey ) );

            if( xResult == CKR_OK )
            {
                xKeyIsInitialized = CK_TRUE;
            }
        }

        xSemaphoreGive( xSessionLock );
    }

    /* Lock the shared crypto session. */
    xSemaphoreTake( xSessionLock, portMAX_DELAY );

    /* Start a hash. */
    if( CKR_OK == xResult )
    {
        xMechSha256.mechanism = CKM_SHA256;
        xResult = pxPkcs11FunctionList->C_DigestInit( xPkcs11Session, &xMechSha256 );
    }

    /* Hash the seed. */
    if( CKR_OK == xResult )
    {
        xResult = pxPkcs11FunctionList->C_DigestUpdate( xPkcs11Session,
                                                        ( CK_BYTE_PTR ) &ullKey,
                                                        sizeof( ullKey ) );
    }

    /* Hash the source address. */
    if( CKR_OK == xResult )
    {
        xResult = pxPkcs11FunctionList->C_DigestUpdate( xPkcs11Session,
                                                        ( CK_BYTE_PTR ) &ulSourceAddress,
                                                        sizeof( ulSourceAddress ) );
    }

    /* Hash the source port. */
    if( CKR_OK == xResult )
    {
        xResult = pxPkcs11FunctionList->C_DigestUpdate( xPkcs11Session,
                                                        ( CK_BYTE_PTR ) &usSourcePort,
                                                        sizeof( usSourcePort ) );
    }

    /* Hash the destination address. */
    if( CKR_OK == xResult )
    {
        xResult = pxPkcs11FunctionList->C_DigestUpdate( xPkcs11Session,
                                                        ( CK_BYTE_PTR ) &ulDestinationAddress,
                                                        sizeof( ulDestinationAddress ) );
    }

    /* Hash the destination port. */
    if( CKR_OK == xResult )
    {
        xResult = pxPkcs11FunctionList->C_DigestUpdate( xPkcs11Session,
                                                        ( CK_BYTE_PTR ) &usDestinationPort,
                                                        sizeof( usDestinationPort ) );
    }

    /* Get the hash. */
    if( CKR_OK == xResult )
    {
        xResult = pxPkcs11FunctionList->C_DigestFinal( xPkcs11Session,
                                                       ucSha256Result,
                                                       &ulLength );
    }

    xSemaphoreGive( xSessionLock );

    /* Use the first four bytes of the hash result as the starting point for
     * all initial sequence numbers for connections based on the input 4-tuple. */
    if( CKR_OK == xResult )
    {
        memcpy( &ulNextSequenceNumber,
                ucSha256Result,
                sizeof( ulNextSequenceNumber ) );

        /* Add the tick count of four-tick intervals. In theory, per the RFC
         * (see above), this approach still allows server equipment to optimize
         * handling of connections from the same device that haven't fully timed out. */
        ulNextSequenceNumber += xTaskGetTickCount() / 4;
    }

    return ulNextSequenceNumber;
}

BaseType_t xApplicationGetRandomNumber( uint32_t * pulNumber )
{
    CK_RV xResult = 0;
    SemaphoreHandle_t xSessionLock = NULL;
    CK_SESSION_HANDLE xPkcs11Session = 0;
    CK_FUNCTION_LIST_PTR pxPkcs11FunctionList = NULL;
    uint32_t ulRandomValue = 0;
    BaseType_t xReturn; /* Return pdTRUE if successful */

    xResult = prvSocketsGetCryptoSession( &xSessionLock,
                                          &xPkcs11Session,
                                          &pxPkcs11FunctionList );

    if( 0 == xResult )
    {
        /* Request a sequence of cryptographically random byte values using
         * PKCS#11. */
        xResult = pxPkcs11FunctionList->C_GenerateRandom( xPkcs11Session,
                                                          ( CK_BYTE_PTR ) &ulRandomValue,
                                                          sizeof( ulRandomValue ) );
    }

    /* Check if any of the API calls failed. */
    if( 0 == xResult )
    {
        xReturn = pdTRUE;
        *( pulNumber ) = ulRandomValue;
    }
    else
    {
        xReturn = pdFALSE;
        *( pulNumber ) = 0uL;
    }

    return xReturn;
}
