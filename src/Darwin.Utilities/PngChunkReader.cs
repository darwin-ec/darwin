// Based on the StackOverflow answer by kulin at
// https://stackoverflow.com/questions/26225373/edit-png-metadata-add-itxt-value
// Original license CC BY-SA 3.0 https://creativecommons.org/licenses/by-sa/3.0/

// This file is part of DARWIN.
// Copyright (C) 1994 - 2020
//
// DARWIN is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// DARWIN is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with DARWIN.  If not, see<https://www.gnu.org/licenses/>.

using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Security.Cryptography.X509Certificates;
using System.Text;
using Darwin.Utilities.Cryptography;

namespace Darwin.Utilities
{
    public class PngChunkReader
    {
        private readonly byte[] _header;

        public IList<Chunk> Chunks { get; }

        public PngChunkReader(string filename)
        {
            _header = new byte[8];
            Chunks = new List<Chunk>();

            using (var filestream = new FileStream(filename, FileMode.Open, FileAccess.Read))
            {
                using (var memoryStream = new MemoryStream())
                {
                    if (filestream == null)
                        throw new ArgumentException("invalid file");

                    filestream.CopyTo(memoryStream);

                    memoryStream.Seek(0, SeekOrigin.Begin);

                    memoryStream.Read(_header, 0, _header.Length);

                    while (memoryStream.Position < memoryStream.Length)
                        Chunks.Add(ChunkFromStream(memoryStream));

                    memoryStream.Close();
                }
            }
        }

        public void AddInternationalText(string keyword, string text)
        {
            // 1-79     (keyword)
            // 1        (null character)
            // 1        (compression flag)
            // 1        (compression method)
            // 0+       (language)
            // 1        (null character)
            // 0+       (translated keyword)
            // 1        (null character)
            // 0+       (text)

            var typeBytes = Encoding.UTF8.GetBytes("iTXt");
            var keywordBytes = Encoding.UTF8.GetBytes(keyword);
            var textBytes = Encoding.UTF8.GetBytes(text);
            var nullByte = BitConverter.GetBytes('\0')[0];
            var zeroByte = BitConverter.GetBytes(0)[0];

            var data = new List<byte>();

            data.AddRange(keywordBytes);
            data.Add(nullByte);
            data.Add(zeroByte);
            data.Add(zeroByte);
            data.Add(nullByte);
            data.Add(nullByte);
            data.AddRange(textBytes);

            var chunk = new Chunk(typeBytes, data.ToArray());

            Chunks.Insert(1, chunk);
        }

        public byte[] ToBytes()
        {
            using (var stream = new MemoryStream())
            {
                stream.Write(_header, 0, _header.Length);

                foreach (var chunk in Chunks)
                    chunk.WriteToStream(stream);

                var bytes = stream.ToArray();

                stream.Close();

                return bytes;
            }
        }

        private static Chunk ChunkFromStream(Stream stream)
        {
            var length = ReadBytes(stream, 4);
            var type = ReadBytes(stream, 4);
            var data = ReadBytes(stream, Convert.ToInt32(BitConverter.ToUInt32(length.Reverse().ToArray(), 0)));

            stream.Seek(4, SeekOrigin.Current);

            return new Chunk(type, data);
        }

        private static byte[] ReadBytes(Stream stream, int n)
        {
            var buffer = new byte[n];
            stream.Read(buffer, 0, n);
            return buffer;
        }

        private static void WriteBytes(Stream stream, byte[] bytes)
        {
            stream.Write(bytes, 0, bytes.Length);
        }

        public class Chunk
        {
            public Chunk(byte[] type, byte[] data)
            {
                Type = type;
                Data = data;
            }

            public void WriteToStream(Stream stream)
            {
                WriteBytes(stream, BitConverter.GetBytes(Convert.ToUInt32(Data.Length)).Reverse().ToArray());
                WriteBytes(stream, Type);
                WriteBytes(stream, Data);
                WriteBytes(stream, CalculateCrc(Type, Data));
            }

            private static byte[] CalculateCrc(IEnumerable<byte> type, IEnumerable<byte> data)
            {
                var bytes = new List<byte>();

                bytes.AddRange(type);
                bytes.AddRange(data);

                var hasher = new Crc32();

                using (var stream = new MemoryStream(bytes.ToArray()))
                    return hasher.ComputeHash(stream);
            }

            public byte[] Type { get; }

            public byte[] Data { get; }
        }
    }
}
