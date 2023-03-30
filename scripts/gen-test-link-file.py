#!/bin/python3

b = bytearray()

b.extend((3).to_bytes(2, "little"))
b.extend((2).to_bytes(2, "little"))
b.extend((14).to_bytes(2, "little"))

b.append(10)

b.extend(b"first")
b.append(0)
b.extend((10).to_bytes(2, "little"))
b.extend(b"D")

b.extend(b"second")
b.append(0)
b.extend((20).to_bytes(2, "little"))
b.extend(b"U")

b.extend(b"third")
b.append(0)
b.extend((30).to_bytes(2, "little"))
b.extend(b"D")

b.append(10)

b.extend((15).to_bytes(2, "little"))
b.extend((1).to_bytes(2, "little"))
b.extend((35).to_bytes(2, "little"))
b.extend((3).to_bytes(2, "little"))

b.append(10)

b.extend(b"hello, world")
b.append(10)
b.append(0)

with open("test-link.o", "wb") as f:
    f.write(b)
