def int_to_hex(value):
    return f"0x{value:08x}"

N = 30
debug = int(input("Enter 1 for debug mode, 0 for normal mode: "))
base_A = 0x00000000
base_B = 0x00000100
base_C = 0x00000200

def get_address(base, i, j, N):
    return base + (i * N + j) * 4

A = [
    [N * i + j + 1 for j in range(N)] for i in range(N)
]

B = [
    [1 if i == j else 0 for j in range(N)] for i in range(N)
]

print("Matrix A:")
for row in A:
    print(row)

print("\nMatrix B:")
for row in B:
    print(row)

C = [[0 for _ in range(N)] for _ in range(N)]

csv_entries = []

def make_write_request(addr, wdata):
    csv_entries.append(f"W,{int_to_hex(addr)},{int_to_hex(wdata)}")

def make_read_request(addr, expected):
    if debug == 0: csv_entries.append(f"R,{int_to_hex(addr)},")
    else: csv_entries.append(f"R,{int_to_hex(addr)},{int_to_hex(expected)}")


# Initialize memory with A, B, C
for i in range(N):
    for j in range(N):
        addr_A = get_address(base_A, i, j, N)
        addr_B = get_address(base_B, i, j, N)
        addr_C = get_address(base_C, i, j, N)
        value_A = A[i][j]
        value_B = B[i][j]
        value_C = C[i][j]
        make_write_request(addr_A, value_A)
        make_write_request(addr_B, value_B)
        make_write_request(addr_C, value_C)

# Simulate matrix multiplication
for i in range(N):
    for j in range(N):
        addr_C = get_address(base_C, i, j, N)
        C[i][j] = 0  # Reset C[i][j] in simulation
        for k in range(N):
            addr_A = get_address(base_A, i, k, N)
            addr_B = get_address(base_B, k, j, N)
            
            value_A = A[i][k]
            make_read_request(addr_A, value_A)
            
            value_B = B[k][j]
            make_read_request(addr_B, value_B)
            
            C[i][j] += value_A * value_B
        
        make_write_request(addr_C, C[i][j])

# Read the result matrix C
for i in range(N):
    for j in range(N):
        addr_C = get_address(base_C, i, j, N)
        value_C = C[i][j]
        make_read_request(addr_C, value_C)
        
# Write to CSV file
with open("memory_requests.csv", "w") as f:
    for entry in csv_entries:
        f.write(entry + "\n")

print("CSV file 'memory_requests.csv' has been generated.")