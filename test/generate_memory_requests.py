def int_to_hex(value):
    return f"0x{value:08x}"

N = int(input("Enter the size of the matrices: "))
test = int(input("Enter 1 for test mode, 0 for normal mode: "))

base_A = 0x00000000
base_B = base_A + N*N*4
base_C = base_B + N*N*4

print(f"Base address for A: {int_to_hex(base_A)}")
print(f"Base address for B: {int_to_hex(base_B)}")
print(f"Base address for C: {int_to_hex(base_C)}")

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

global write_requests
global read_requests

write_requests = 0
read_requests = 0

def make_write_request(addr, wdata):
    csv_entries.append(f"W,{int_to_hex(addr)},{int_to_hex(wdata)}")

def make_read_request(addr, expected):
    if test == 0: csv_entries.append(f"R,{int_to_hex(addr)},")
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
        write_requests += 3  # Count the writes for A, B, C

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
            read_requests += 2
        
        make_write_request(addr_C, C[i][j])
        write_requests += 1

# Read the result matrix C
for i in range(N):
    for j in range(N):
        addr_C = get_address(base_C, i, j, N)
        value_C = C[i][j]
        make_read_request(addr_C, value_C)
        read_requests += 1
        


# Write to CSV file
with open("../test.csv" if test else "../requests.csv", "w") as f:
    f.write("\n".join(csv_entries))

print("CSV file 'memory_requests.csv' has been generated. Total write requests: ", write_requests, " Total read requests:", read_requests)