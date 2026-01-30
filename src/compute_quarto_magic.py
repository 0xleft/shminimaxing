magics = [
    0x8888,  # vertical
    0xf000  # horizontal
]

computed_full_magics = [
    '0x8421',  # positive
    '0x1248'  # negative
]

# horizontal compute
for i in range(4):
    computed_full_magics.append(hex(magics[0] >> i))

# vertical compute
for i in range(4):
    computed_full_magics.append(hex(magics[1] >> (i * 4)))

print(computed_full_magics)

print(",".join(computed_full_magics))
