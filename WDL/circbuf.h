/*
    WDL - circbuf.h
    Copyright (C) 2005 Cockos Incorporated

    This software is provided 'as-is', without any express or implied
    warranty.  In no event will the authors be held liable for any damages
    arising from the use of this software.

    Permission is granted to anyone to use this software for any purpose,
    including commercial applications, and to alter it and redistribute it
    freely, subject to the following restrictions:

    1. The origin of this software must not be misrepresented; you must not
       claim that you wrote the original software. If you use this software
       in a product, an acknowledgment in the product documentation would be
       appreciated but is not required.
    2. Altered source versions must be plainly marked as such, and must not be
       misrepresented as being the original software.
    3. This notice may not be removed or altered from any source distribution.
    
*/

/*

  This file provides a simple class for a circular FIFO queue of bytes. It 
  has a strong advantage over WDL_Queue with large buffers, as it does far 
  fewer memcpy()'s.

*/

#ifndef _WDL_CIRCBUF_H_
#define _WDL_CIRCBUF_H_

#include "heapbuf.h"

class WDL_CircBuf
{
public:
  WDL_CircBuf()
  {
    m_size = 0;
    m_inbuf = 0;
    m_head = m_tail = 0;
  }
  ~WDL_CircBuf()
  {
  }
  void SetSize(int size, bool keepcontents=false)
  {
    WDL_HeapBuf tmp;
    if (keepcontents && size && NbInBuf()) 
    {
      tmp.Resize(min(NbInBuf(),size));
      if (tmp.GetSize()) Get(tmp.Get(),tmp.GetSize());
    }
    m_size = size;
    m_hb.Resize(size);
    Reset();
    if (tmp.GetSize()) Add(tmp.Get(),tmp.GetSize()); // add old data back in
  }
  void Reset()
  {
    m_head = (char *)m_hb.Get();
    m_tail = (char *)m_hb.Get();
    m_endbuf = (char *)m_hb.Get() + m_size;
    m_inbuf = 0;
  }
  int Add(const void *buf, int l)
  {
    char *p = (char *)buf;
    int put = l;
    int l2;
    if (!m_size) return 0;
    l2 = m_endbuf - m_head;
    if (l2 <= l) 
    {
      memcpy(m_head, p, l2);
      m_head = (char *)m_hb.Get();
      p += l2;
      l -= l2;
    }
    if (l) 
    {
      memcpy(m_head, p, l);
      m_head += l;
      p += l;
    }
    m_inbuf += put;
    return put;
  }
  int Get(void *buf, int l)
  {
    char *p = (char *)buf;
    int got = 0;
    if (!m_size) return 0;
    if (m_inbuf <= 0) return 0;
    if (l > m_inbuf) l = m_inbuf;
    m_inbuf -= l;
    got = l;
    if (m_tail+l >= m_endbuf) 
    {
      int l1 = m_endbuf - m_tail;
      l -= l1;
      memcpy(p, m_tail, l1);
      m_tail = (char *)m_hb.Get();
      p += l1;
      memcpy(p, m_tail, l);
      m_tail += l;
    } 
    else 
    {
      memcpy(p, m_tail, l);
      m_tail += l;
    }
    return got;
  }
  int Available() { return m_size - m_inbuf; }
  int NbInBuf() { return m_inbuf; }

private:
  WDL_HeapBuf m_hb;
  char *m_head, *m_tail, *m_endbuf;
  int m_size, m_inbuf;
};

#endif