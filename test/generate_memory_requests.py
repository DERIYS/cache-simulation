# This script generates memory requests for matrix multiplication
# It creates a CSV file with read and write requests for matrices A, B, and C, simulating the memory operations 
# of a matrix multiplication algorithm.
#
# It can be run in two modes: test mode (generates expected read data) and normal mode (no expected data)
# The generated CSV file can be used to test the cache simulation on real requests from a real algorithm

def int_to_hex(value):
    return f"0x{value:08x}"

N = int(input("Enter the size of the matrices: "))
test = int(input("Enter 1 for test mode (generates expected read data with read requests), 0 for normal mode: "))

base_A = 0x00000000
base_B = base_A + N*N*4
base_C = base_B + N*N*4

def get_address(base, i, j, N):
    return base + (i * N + j) * 4

A = [[N * i + j + 1 for j in range(N)] for i in range(N)]

B = [[1 if i == j else 0 for j in range(N)] for i in range(N)] # Identity matrix for simplicity

C = [[0 for _ in range(N)] for _ in range(N)] # Result matrix initialized to zero

csv_entries = []

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
        
filename = "test.csv" if test else "requests.csv"

# Write to CSV file
with open("../" + filename, "w") as f:
    f.write("\n".join(csv_entries))

print(f"CSV file '{filename}' in the repository's root has been generated. Total requests: {write_requests + read_requests} ({read_requests} reads and {write_requests} writes)\n\n" + 
      "Simply rerun the simulation with \"make run-test\" or \"make run\" command depending on which option you chose before.\n")