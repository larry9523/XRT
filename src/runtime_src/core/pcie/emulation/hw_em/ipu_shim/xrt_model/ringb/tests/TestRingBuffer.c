/*
* SPDX-License-Identifier: Apache-2.0
* Copyright(C) 2021 Advanced Micro Devices, Inc. All rights reserved.
* Copyright (C) 2022 Advanced Micro Devices, Inc. All rights reserved.
*/

#include "unity.h"
#include "ring_buffer.h"
#include <stdio.h>
#include <string.h>

#define TEST_RING_SIZE 0x80
uint8_t TestBuffer[ TEST_RING_SIZE ];

RingBuffer_t TestRingBuffer;

//#define DEBUG_TRACE do{ fprintf( stderr, "[TRACE] %s:%i\n", __FUNCTION__,__LINE__ ); fflush( stderr ); }while (0)

/**
 * Print the ring buffer context information for debugging failing/misbehaving
 * tests
 *
 * @param pRing  The ring buffer pointer
 * @param action String description of the action that resulted in the current
 *               context
 * @param line   The line number
 */
void printContext(RingBuffer_t *pRing, const char *action, int line)
{
    printf("%s at %i: buffer(0x%p)|head(0x%X)|tail(0x%X)|tombstone(0x%X)\n"
        , action
        , line
        , pRing->buffer
        , pRing->Context.Head
        , pRing->Context.Tail
        , pRing->Context.Tombstone);
}
/**
 * Helper macro to print the context
 */
#define PRINT_CONTEXT( A ) printContext( &RxRing, #A, __LINE__ )

void setUp(void)
{
    TestRingBuffer.buffer = TestBuffer;
    TestRingBuffer.buffer_size = sizeof( TestBuffer );
    int initResult = RINGB_Init(&TestRingBuffer);
    TEST_ASSERT_EQUAL(RBS_OK, initResult);
}

void tearDown(void)
{
}

/**
 * Print a byte buffer, one byte at a time - for debugging
 *
 * @param pBuf   Pointer to buffer to print
 * @param bufLen Number of bytes to print
 */
void printUint8Buffer(uint8_t *pBuf, uint16_t bufLen)
{
    uint32_t bufIndex;
    for (bufIndex = 0; bufIndex < bufLen; ++bufIndex)
    {
        printf("0x%02X: 0x%02X \n", bufIndex, pBuf[bufIndex]);
    }
}

static void testNullParams()
{
    RingBufferStatus_t initResult = RINGB_Init(NULL);
    TEST_ASSERT_NOT_EQUAL_INT(RBS_OK, initResult);
}

static void testInsertNullParams()
{
    uint16_t insertData = RINGB_Insert(NULL, NULL, 1);
    TEST_ASSERT_EQUAL(0, insertData);
}

static void testGetDataNullParams()
{
    uint16_t outDataLen = RINGB_GetData(NULL, NULL);
    TEST_ASSERT_EQUAL(0, outDataLen);
}

static void testPopHeadNullParams()
{
    RingBufferStatus_t status = RINGB_PopHead(NULL);
    TEST_ASSERT_NOT_EQUAL(RBS_OK, status);
}

static void testIsEmptyNullParams()
{
    int result = RINGB_IsEmpty(NULL);
    TEST_ASSERT_EQUAL(0, result);
}

static void testInsertSize()
{
    uint8_t buffer[ ] = { 1, 2, 3, 7 };

    size_t insSize = RINGB_Insert(&TestRingBuffer, buffer, sizeof( buffer ));
    TEST_ASSERT_EQUAL(sizeof( buffer ), insSize);
}

static void testInsertTailMoved()
{
    uint8_t buffer[ ] = { 1, 2, 3, 7 };

    uint32_t CurrentTail = TestRingBuffer.Context.Tail;

    RINGB_Insert(&TestRingBuffer, buffer, sizeof( buffer ));
    uint32_t NewTail = TestRingBuffer.Context.Tail;
    TEST_ASSERT_NOT_EQUAL(CurrentTail, NewTail);
}

static void testGetData()
{
    uint8_t buffer[ ] = { 1, 2, 3, 7 };
    int x;

    uint32_t CurrentTail = TestRingBuffer.Context.Tail;

    RINGB_Insert(&TestRingBuffer, buffer, sizeof( buffer ));
    uint32_t NewTail = TestRingBuffer.Context.Tail;
    TEST_ASSERT_NOT_EQUAL(CurrentTail, NewTail);

    uint8_t *outBuffer = NULL;
    uint16_t outDataSize = RINGB_GetData(&TestRingBuffer, (void **)&outBuffer);
    TEST_ASSERT_EQUAL(sizeof( buffer ), outDataSize);
    TEST_ASSERT_NOT_NULL(outBuffer);
    for (x = 0; x < sizeof( buffer ); ++x)
    {
        TEST_ASSERT_EQUAL(buffer[x], outBuffer[x]);
    }
}

static void testPopHead()
{
    uint8_t buffer1[  ] = {
        0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8, 0x9, 0xA, 0xB, 0xC, 0xD, 0xE, 0xF,
    };
    uint8_t buffer2[  ] = {
        0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8, 0x9, 0xA, 0xB, 0xC, 0xD, 0xE, 0xF,
        0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8, 0x9, 0xA, 0xB, 0xC, 0xD, 0xE, 0xF,
        0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8, 0x9, 0xA, 0xB, 0xC, 0xD, 0xE, 0xF,
    };
    uint8_t *data = NULL;
    uint16_t insertLen = 0;

    insertLen = RINGB_Insert(&TestRingBuffer, buffer1, sizeof( buffer1 ));
    TEST_ASSERT_EQUAL(sizeof( buffer1 ), insertLen);
    insertLen = RINGB_Insert(&TestRingBuffer, buffer1, sizeof( buffer1 ));
    TEST_ASSERT_EQUAL(sizeof( buffer1 ), insertLen);
    insertLen = RINGB_Insert(&TestRingBuffer, buffer2, sizeof( buffer2 ));
    TEST_ASSERT_EQUAL(sizeof( buffer2 ), insertLen);

    uint16_t getDataLen = RINGB_GetData(&TestRingBuffer, (void **)&data);
    uint8_t *pOut = (uint8_t *)(data);

    TEST_ASSERT_EQUAL(sizeof( buffer1 ), getDataLen);
    TEST_ASSERT_EQUAL_UINT8_ARRAY(buffer1, pOut, sizeof( buffer1 ));
    TEST_ASSERT_EQUAL(TestRingBuffer.buffer, &TestRingBuffer.buffer[TestRingBuffer.Context.Head]);

    RingBufferStatus_t status = RINGB_PopHead(&TestRingBuffer);
    TEST_ASSERT_EQUAL(RBS_OK, status);
    TEST_ASSERT_NOT_EQUAL(TestRingBuffer.buffer, &TestRingBuffer.buffer[TestRingBuffer.Context.Head]);
}

static void testInsertWrap()
{
    uint8_t buf4[ sizeof( TestBuffer ) / 4 ] = { 0xFF };
    uint8_t buf2[ sizeof( TestBuffer ) / 8 ] = { 0xFF };
    uint8_t *data = NULL;
    uint16_t outDataLen = 0;

    //////////////    Add/Remove 1
    RINGB_Insert(&TestRingBuffer, buf2, sizeof( buf2 ));
    outDataLen = RINGB_GetData(&TestRingBuffer, (void **)&data);
    TEST_ASSERT_EQUAL(sizeof( buf2 ), outDataLen);
    TEST_ASSERT_EQUAL_UINT8_ARRAY(buf2, (uint8_t *)data, sizeof( buf2 ));
    TEST_ASSERT_GREATER_THAN(TestRingBuffer.Context.Head, TestRingBuffer.Context.Tail);
    TEST_ASSERT_EQUAL(TestRingBuffer.buffer_size, TestRingBuffer.Context.Tombstone);

    RINGB_PopHead(&TestRingBuffer);
    int empty = RINGB_IsEmpty(&TestRingBuffer);
    TEST_ASSERT_NOT_EQUAL(0, empty);

    //////////////    Add/Remove 2
    RINGB_Insert(&TestRingBuffer, buf4, sizeof( buf4 ));
    outDataLen = RINGB_GetData(&TestRingBuffer, (void **)&data);
    TEST_ASSERT_EQUAL(sizeof( buf4 ), outDataLen);
    TEST_ASSERT_EQUAL_UINT8_ARRAY(buf4, (uint8_t *)data, sizeof( buf4 ));
    TEST_ASSERT_GREATER_THAN(TestRingBuffer.Context.Head, TestRingBuffer.Context.Tail);
    TEST_ASSERT_EQUAL(TestRingBuffer.buffer_size, TestRingBuffer.Context.Tombstone);

    RINGB_PopHead(&TestRingBuffer);
    empty = RINGB_IsEmpty(&TestRingBuffer);
    TEST_ASSERT_NOT_EQUAL(0, empty);

    //////////////    Add/Remove 3
    RINGB_Insert(&TestRingBuffer, buf4, sizeof( buf4 ));
    TEST_ASSERT_GREATER_THAN(TestRingBuffer.Context.Head, TestRingBuffer.Context.Tail);
    TEST_ASSERT_EQUAL(TestRingBuffer.buffer_size, TestRingBuffer.Context.Tombstone);

    RINGB_PopHead(&TestRingBuffer);
    TEST_ASSERT_EQUAL(TestRingBuffer.Context.Head, TestRingBuffer.Context.Tail);

    //////////////    Add/Remove 4
    RINGB_Insert(&TestRingBuffer, buf4, sizeof( buf4 ));
    TEST_ASSERT_NOT_EQUAL(TestRingBuffer.Context.Head, TestRingBuffer.Context.Tail);
    TEST_ASSERT_GREATER_THAN(TestRingBuffer.Context.Head, TestRingBuffer.Context.Tail);
    TEST_ASSERT_EQUAL(TestRingBuffer.buffer_size, TestRingBuffer.Context.Tombstone);
    RINGB_PopHead(&TestRingBuffer);
    TEST_ASSERT_EQUAL(TestRingBuffer.buffer_size, TestRingBuffer.Context.Tombstone);

    // Throw in a check that trying to pop the head when empty returns a NO DATA error, but doesn't break anything else
    RingBufferStatus_t status = RINGB_PopHead(&TestRingBuffer);
    TEST_ASSERT_EQUAL(RBS_NO_DATA, status);
    TEST_ASSERT_EQUAL(TestRingBuffer.buffer_size, TestRingBuffer.Context.Tombstone);

    // Buffer should not be able to hold 4 buf4 without rolling over (plus, we added the buf2 at the beginning
    uint16_t insertLen = RINGB_Insert(&TestRingBuffer, buf4, sizeof( buf4 ));
    TEST_ASSERT_NOT_EQUAL(TestRingBuffer.buffer_size, TestRingBuffer.Context.Tombstone);
    TEST_ASSERT_EQUAL(sizeof( buf4 ), insertLen);
    // Note that we not check that tombstone is NOT NULL
    TEST_ASSERT_NOT_EQUAL(TestRingBuffer.buffer_size, TestRingBuffer.Context.Tombstone);
    // Note that we make sure pHead is now greater than pTail
    TEST_ASSERT_GREATER_THAN(TestRingBuffer.Context.Tail, TestRingBuffer.Context.Head);
    // Now, when we get the data, it will see that the insert rolled over and will update the pHead and pTombstone
    // pointers
    outDataLen = RINGB_GetData(&TestRingBuffer, (void **)&data);
    // Now we make sure the tombstone is removed
    TEST_ASSERT_EQUAL(TestRingBuffer.buffer_size, TestRingBuffer.Context.Tombstone);
    TEST_ASSERT_EQUAL(sizeof( buf4 ), outDataLen);
    TEST_ASSERT_GREATER_THAN(TestRingBuffer.Context.Head, TestRingBuffer.Context.Tail);
    TEST_ASSERT_EQUAL_UINT8_ARRAY(buf4, (uint8_t *)data, sizeof( buf4 ));

    RINGB_PopHead(&TestRingBuffer);
    empty = RINGB_IsEmpty(&TestRingBuffer);
    TEST_ASSERT_NOT_EQUAL(0, empty);
}

static void testBufferFull()
{
    uint8_t bigBuff[ (sizeof( TestBuffer ) / 4) * 3 ];
    int bufIndex;

    for (bufIndex = 0; bufIndex < sizeof( bigBuff ); ++bufIndex)
    {
        bigBuff[bufIndex] = (uint8_t)bufIndex;
    }

    // store
    uint16_t insertLen = RINGB_Insert(&TestRingBuffer, bigBuff, sizeof( bigBuff ));
    TEST_ASSERT_EQUAL(sizeof( bigBuff ), insertLen);
    // full
    insertLen = RINGB_Insert(&TestRingBuffer, bigBuff, sizeof( bigBuff ));
    TEST_ASSERT_EQUAL(0, insertLen);

    uint8_t *data;

    uint16_t outDataLen = RINGB_GetData(&TestRingBuffer, (void *)&data);
    TEST_ASSERT_EQUAL(sizeof( bigBuff ), outDataLen);
    TEST_ASSERT_EQUAL_UINT8_ARRAY(bigBuff, (uint8_t *)data, sizeof( bigBuff ));

    // empty
    RINGB_PopHead(&TestRingBuffer);
    int empty = RINGB_IsEmpty(&TestRingBuffer);
    TEST_ASSERT_NOT_EQUAL(0, empty);

    // store
    insertLen = RINGB_Insert(&TestRingBuffer, bigBuff, sizeof( bigBuff ));
    TEST_ASSERT_EQUAL(sizeof( bigBuff ), insertLen);
}

// not just full, but EVERY available byte is used
static void testSaturatedBuffer()
{
    uint8_t buff[ (TEST_RING_SIZE / 4) - 2 ] = { 0 };
    memset(buff, 0xFF, sizeof( buff ));
    uint16_t insertSize;
    insertSize = RINGB_Insert(&TestRingBuffer, buff, sizeof( buff ));
    TEST_ASSERT_EQUAL(sizeof( buff ), insertSize);
    insertSize = RINGB_Insert(&TestRingBuffer, buff, sizeof( buff ));
    TEST_ASSERT_EQUAL(sizeof( buff ), insertSize);
    insertSize = RINGB_Insert(&TestRingBuffer, buff, sizeof( buff ));
    TEST_ASSERT_EQUAL(sizeof( buff ), insertSize);
    insertSize = RINGB_Insert(&TestRingBuffer, buff, sizeof( buff ));
    TEST_ASSERT_EQUAL(sizeof( buff ), insertSize);

    int empty = RINGB_IsEmpty(&TestRingBuffer);
    TEST_ASSERT_EQUAL(0, empty);
}

// make sure tail behavior is correct when we use exactly the available bytes at the end of the buffer
static void testWrapOnExactFillAtEnd()
{
    uint8_t buff[ (TEST_RING_SIZE / 4) - 2 ] = { 0 };
    uint16_t insertSize;
    insertSize = RINGB_Insert(&TestRingBuffer, buff, sizeof( buff ));
    TEST_ASSERT_EQUAL(sizeof( buff ), insertSize);

    RingBufferStatus_t status = RINGB_PopHead(&TestRingBuffer);
    TEST_ASSERT_EQUAL(RBS_OK, status);
    int empty = RINGB_IsEmpty(&TestRingBuffer);
    TEST_ASSERT_EQUAL(1, empty);

    insertSize = RINGB_Insert(&TestRingBuffer, buff, sizeof( buff ));
    TEST_ASSERT_EQUAL(sizeof( buff ), insertSize);
    insertSize = RINGB_Insert(&TestRingBuffer, buff, sizeof( buff ));
    TEST_ASSERT_EQUAL(sizeof( buff ), insertSize);
    insertSize = RINGB_Insert(&TestRingBuffer, buff, sizeof( buff ));
    TEST_ASSERT_EQUAL(sizeof( buff ), insertSize);
    insertSize = RINGB_Insert(&TestRingBuffer, buff, sizeof( buff ));
    TEST_ASSERT_EQUAL(sizeof( buff ), insertSize);

    empty = RINGB_IsEmpty(&TestRingBuffer);
    TEST_ASSERT_EQUAL(0, empty);
}

// make sure we handle when we use exactly the available bytes between tail and head when we are in a wrapped condition
static void testWrappedExactFill()
{
    uint8_t buff[ (TEST_RING_SIZE / 4) - 2 ] = { 0 };
    uint16_t insertSize;

    // Add two block == half the buffer
    insertSize = RINGB_Insert(&TestRingBuffer, buff, sizeof( buff ));
    TEST_ASSERT_EQUAL_UINT16(sizeof( buff ), insertSize);
    insertSize = RINGB_Insert(&TestRingBuffer, buff, sizeof( buff ));
    TEST_ASSERT_EQUAL_UINT16(sizeof( buff ), insertSize);

    // Remove two blocks
    RingBufferStatus_t status = RINGB_PopHead(&TestRingBuffer);
    TEST_ASSERT_EQUAL_INT(RBS_OK, status);
    status = RINGB_PopHead(&TestRingBuffer);
    TEST_ASSERT_EQUAL_INT(RBS_OK, status);

    // Add three blocks
    insertSize = RINGB_Insert(&TestRingBuffer, buff, sizeof( buff ));
    TEST_ASSERT_EQUAL_UINT16(sizeof( buff ), insertSize);
    insertSize = RINGB_Insert(&TestRingBuffer, buff, sizeof( buff ));
    TEST_ASSERT_EQUAL_UINT16(sizeof( buff ), insertSize);
    insertSize = RINGB_Insert(&TestRingBuffer, buff, sizeof( buff ));
    TEST_ASSERT_EQUAL_UINT16(sizeof( buff ), insertSize);
}

// iterate a relatively huge number of times writing and reading some values to see if we can conjure up any other
// failure
static void testManyIterations()
{
#define TEST_BUFFER_SIZE  0x03
    uint8_t buff[ TEST_BUFFER_SIZE ] = { 0 };
    uint8_t *data = NULL;
    uint32_t iterIndex;

    for (iterIndex = 0; iterIndex < 0x1000000; ++iterIndex)
    {
        uint32_t bufIndex;
        for (bufIndex = 0; bufIndex < TEST_BUFFER_SIZE; ++bufIndex)
        {
            buff[bufIndex] = (uint8_t)iterIndex + bufIndex;
        }
        const uint16_t insertLen = RINGB_Insert(&TestRingBuffer, buff, sizeof( buff ));
        TEST_ASSERT_EQUAL(sizeof( buff ), insertLen);
        const uint16_t outLen = RINGB_GetData(&TestRingBuffer, (void **)&data);
        TEST_ASSERT_EQUAL(sizeof( buff ), outLen);
        TEST_ASSERT_EQUAL_UINT8_ARRAY(buff, (uint8_t *)data, sizeof( buff ));
        const RingBufferStatus_t status = RINGB_PopHead(&TestRingBuffer);
        TEST_ASSERT_EQUAL(RBS_OK, status);
        const int empty = RINGB_IsEmpty(&TestRingBuffer);
        TEST_ASSERT_EQUAL(1, empty);
    }
}

int main(int argc, char **argv)
{
    UNITY_BEGIN();
    RUN_TEST(testNullParams);
    RUN_TEST(testInsertNullParams);
    RUN_TEST(testGetDataNullParams);
    RUN_TEST(testPopHeadNullParams);
    RUN_TEST(testIsEmptyNullParams);
    RUN_TEST(testInsertSize);
    RUN_TEST(testInsertTailMoved);
    RUN_TEST(testGetData);
    RUN_TEST(testPopHead);
    RUN_TEST(testInsertWrap);
    RUN_TEST(testBufferFull);
    RUN_TEST(testSaturatedBuffer);
    RUN_TEST(testWrapOnExactFillAtEnd);
    RUN_TEST(testWrappedExactFill);
    RUN_TEST(testManyIterations);
    return UNITY_END();
}

