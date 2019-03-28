#pragma once

/*
    Copyright © 2016, The Network Protocol Company, Inc.

    Redistribution and use in source and binary forms, with or without modification, are permitted
   provided that the following conditions are met:

        1. Redistributions of source code must retain the above copyright notice, this list of
   conditions and the following disclaimer.

        2. Redistributions in binary form must reproduce the above copyright notice, this list of
   conditions and the following disclaimer in the documentation and/or other materials provided with
   the distribution.

        3. Neither the name of the copyright holder nor the names of its contributors may be used to
   endorse or promote products derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR
   IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
   FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
   CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
   DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
   DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
    WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
   ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <assert.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
inline bool sequence_greater_than(uint16_t s1, uint16_t s2)
{
    return ((s1 > s2) && (s1 - s2 <= 32768)) || ((s1 < s2) && (s2 - s1 > 32768));
}

inline bool sequence_less_than(uint16_t s1, uint16_t s2) { return sequence_greater_than(s2, s1); }

// IDK glenn...this is way more complicated than your post on the interwebs
class BitArray
{
  public:
    BitArray(int size)
    {
        assert(size > 0);
        m_size = size;
        m_bytes = 8 * ((size / 64) + ((size % 64) ? 1 : 0));
        assert(m_bytes > 0);
        assert((m_bytes % 8) == 0);
        m_data = new uint64_t[m_bytes / 8];
        Clear();
    }

    ~BitArray()
    {
        assert(m_data);
        delete[] m_data;
        m_data = NULL;
    }

    void Clear()
    {
        assert(m_data);
        memset(m_data, 0, m_bytes);
    }

    void SetBit(int index)
    {
        assert(index >= 0);
        assert(index < m_size);
        const int data_index = index >> 6;
        const int bit_index = index & ((1 << 6) - 1);
        assert(bit_index >= 0);
        assert(bit_index < 64);
        m_data[data_index] |= uint64_t(1) << bit_index;
    }

    void ClearBit(int index)
    {
        assert(index >= 0);
        assert(index < m_size);
        const int data_index = index >> 6;
        const int bit_index = index & ((1 << 6) - 1);
        m_data[data_index] &= ~(uint64_t(1) << bit_index);
    }

    uint64_t GetBit(int index) const
    {
        assert(index >= 0);
        assert(index < m_size);
        const int data_index = index >> 6;
        const int bit_index = index & ((1 << 6) - 1);
        assert(bit_index >= 0);
        assert(bit_index < 64);
        return (m_data[data_index] >> bit_index) & 1;
    }

    int GetSize() const { return m_size; }

  private:
    int m_size;
    int m_bytes;
    uint64_t* m_data;

    BitArray(const BitArray& other);
    BitArray& operator=(const BitArray& other);
};
template <typename T>
class SequenceBuffer
{
  public:
    SequenceBuffer(int size) : m_exists(size)
    {
        assert(size > 0);
        m_size = size;
        m_first_entry = true;
        m_sequence = 0;
        m_entry_sequence = new uint16_t[size];
        m_entry_data = new T[size];
        Reset();
    }

    ~SequenceBuffer()
    {
        assert(m_entry_data);
        assert(m_entry_sequence);
        delete[] m_entry_data;
        delete[] m_entry_sequence;
        m_entry_data = NULL;
        m_entry_sequence = NULL;
    }

    void Reset()
    {
        m_first_entry = true;
        m_sequence = 0;
        m_exists.Clear();
        memset(m_entry_sequence, 0, sizeof(uint16_t) * m_size);
    }

    T* Insert(uint16_t sequence)
    {
        if (m_first_entry)
        {
            m_sequence = sequence + 1;
            m_first_entry = false;
        }
        else if (sequence_less_than(sequence, m_sequence - m_size))
        {
            return NULL;
        }
        else if (sequence_greater_than(sequence + 1, m_sequence))
        {
            m_sequence = sequence + 1;
        }

        const int index = sequence % m_size;

        m_exists.SetBit(index);

        m_entry_sequence[index] = sequence;

        return &m_entry_data[index];
    }

    void Remove(uint16_t sequence) { m_exists.ClearBit(sequence % m_size); }

    void RemoveOldEntries()
    {
        const uint16_t oldest_sequence = m_sequence - m_size;
        for (int i = 0; i < m_size; ++i)
        {
            if (m_exists.GetBit(i) && sequence_less_than(m_entry_sequence[i], oldest_sequence))
                m_exists.ClearBit(i);
        }
    }

    bool IsAvailable(uint16_t sequence) const { return !m_exists.GetBit(sequence % m_size); }

    int GetIndex(uint16_t sequence) const { return sequence % m_size; }

    const T* Find(uint16_t sequence) const
    {
        const int index = sequence % m_size;
        if (m_exists.GetBit(index) && m_entry_sequence[index] == sequence)
            return &m_entry_data[index];
        else
            return NULL;
    }

    T* Find(uint16_t sequence)
    {
        const int index = sequence % m_size;
        if (m_exists.GetBit(index) && m_entry_sequence[index] == sequence)
            return &m_entry_data[index];
        else
            return NULL;
    }

    T* GetAtIndex(int index)
    {
        assert(index >= 0);
        assert(index < m_size);
        return m_exists.GetBit(index) ? &m_entry_data[index] : NULL;
    }

    uint16_t GetSequence() const { return m_sequence; }

    int GetSize() const { return m_size; }

  private:
    T* m_entry_data;
    uint16_t* m_entry_sequence;
    int m_size;
    uint16_t m_sequence;
    bool m_first_entry;
    BitArray m_exists;

    SequenceBuffer(const SequenceBuffer<T>& other);
    SequenceBuffer<T>& operator=(const SequenceBuffer<T>& other);
};

template <typename T>
void GenerateAckBits(const SequenceBuffer<T>& packets, uint16_t& ack, uint32_t& ack_bits)
{
    ack = packets.GetSequence() - 1;
    ack_bits = 0;
    for (int i = 0; i < 32; ++i)
    {
        uint16_t sequence = ack - i;
        if (packets.Find(sequence))
            ack_bits |= (1 << i);
    }
}