#!/bin/python3

b = bytearray()

b.extend((3).to_bytes(2, "little"))
b.extend((2).to_bytes(2, "little"))
b.extend((24).to_bytes(2, "little"))

b.append(10)

b.extend(b"message")
b.append(0)
b.extend((0).to_bytes(2, "little"))
b.append(0)

b.extend(b"_start")
b.append(0)
b.extend((14).to_bytes(2, "little"))
b.append(1)

b.extend(b"strlen")
b.append(0)
b.extend((0xaffa).to_bytes(2, "little"))
b.append(2)

b.append(10)

b.extend((15).to_bytes(2, "little"))
b.extend((0).to_bytes(2, "little"))
b.extend((18).to_bytes(2, "little"))
b.extend((2).to_bytes(2, "little"))

b.append(10)

b.extend(b"hello, world")
b.append(10)
b.append(0)

b.extend((0x89).to_bytes(1, "little"))
b.extend((0xaffa).to_bytes(2, "little"))
b.extend((0xa0).to_bytes(1, "little"))
b.extend((0xaffa).to_bytes(2, "little"))
b.extend((0x9f).to_bytes(1, "little"))
b.extend((1).to_bytes(2, "little"))
b.extend((0x9e).to_bytes(1, "little"))

with open("test-link-1.o", "wb") as f:
    f.write(b)

b = bytearray()

b.extend((3).to_bytes(2, "little"))
b.extend((2).to_bytes(2, "little"))
b.extend((18).to_bytes(2, "little"))

b.append(10)

b.extend(b"strlen")
b.append(0)
b.extend((0).to_bytes(2, "little"))
b.append(1)

b.extend(b"strlen_loop")
b.append(0)
b.extend((3).to_bytes(2, "little"))
b.append(0)

b.extend(b"strlen_done")
b.append(0)
b.extend((17).to_bytes(2, "little"))
b.append(0)

b.append(10)

b.extend((11).to_bytes(2, "little"))
b.extend((2).to_bytes(2, "little"))
b.extend((15).to_bytes(2, "little"))
b.extend((1).to_bytes(2, "little"))

b.append(10)

b.extend((0x89).to_bytes(1, "little"))
b.extend((0x0).to_bytes(2, "little"))
b.extend((0x8b).to_bytes(1, "little"))
b.extend((0x8b).to_bytes(1, "little"))
b.extend((0x82).to_bytes(1, "little"))
b.extend((0x1).to_bytes(1, "little"))
b.extend((0x9).to_bytes(1, "little"))
b.extend((0x0).to_bytes(1, "little"))
b.extend((0x16).to_bytes(1, "little"))
b.extend((0x9d).to_bytes(1, "little"))
b.extend((0xaffa).to_bytes(2, "little"))
b.extend((0x87).to_bytes(1, "little"))
b.extend((0x9c).to_bytes(1, "little"))
b.extend((0xaffa).to_bytes(2, "little"))
b.extend((0xa1).to_bytes(1, "little"))

with open("test-link-2.o", "wb") as f:
    f.write(b)
