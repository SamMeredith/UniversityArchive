#include <stdint.h>
#include <stdio.h>

#define CIRC_BUF_SIZE   256

typedef struct
{
    uint8_t buffer[CIRC_BUF_SIZE];
    uint8_t head;
    uint8_t tail;
    size_t maxlen;
} circ_buf_t;

void circ_buf_init(circ_buf_t *_this);
int circ_buf_get(circ_buf_t *_this, uint8_t *c);
int circ_buf_put(circ_buf_t *_this, uint8_t c);

//------------------------------------------------------
//
//------------------------------------------------------
int main()
{
    circ_buf_t buffer;
    
    circ_buf_init(&buffer);
    
    circ_buf_put(&buffer, 3);
    
    uint8_t res = 0;
    circ_buf_get(&buffer, &res);
    
    printf("\r\nGot %d from buffer", res);
    
    return 0;
}

//------------------------------------------------------
//
//------------------------------------------------------
void circ_buf_init(circ_buf_t *_this)
{
    _this->head = 0;
    _this->tail = 0;
    _this->maxlen = CIRC_BUF_SIZE;

    memset(_this->buffer, 0, _this->maxlen);
}

//------------------------------------------------------
//
//------------------------------------------------------
int circ_buf_get(circ_buf_t *_this, uint8_t *c)
{
    int next; // Index that tail will point to after this get

    if (_this->head == _this->tail) // Circular buffer is empty
        return -1;

    next = _this->tail + 1;

    if (next >= _this->maxlen)
        next = 0;

    *c = _this->buffer[_this->tail];
    _this->tail = next;

    return 0;
}

//------------------------------------------------------
//
//------------------------------------------------------
int circ_buf_put(circ_buf_t *_this, uint8_t c)
{
    int next; // Index that head will point to after this put

    next = _this->head + 1;

    if (next >= _this->maxlen)
        next = 0;

    if (next == _this->tail) // If head + 1 == tail, circular buffer is full
        return -1;

    _this->buffer[_this->head] = c;
    _this->head = next;

    return 0;
}