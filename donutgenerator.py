import math

R = 1.0   
r = 0.3  
N = 32    
M = 16    

vertices = []
faces = []

# generate vertices
for i in range(N):
    theta = 2 * math.pi * i / N
    cos_theta, sin_theta = math.cos(theta), math.sin(theta)
    for j in range(M):
        phi = 2 * math.pi * j / M
        cos_phi, sin_phi = math.cos(phi), math.sin(phi)
        x = (R + r * cos_phi) * cos_theta
        y = (R + r * cos_phi) * sin_theta
        z = r * sin_phi
        vertices.append((x, y, z))

# generate faces
for i in range(N):
    for j in range(M):
        next_i = (i + 1) % N
        next_j = (j + 1) % M
        v0 = i * M + j + 1
        v1 = next_i * M + j + 1
        v2 = next_i * M + next_j + 1
        v3 = i * M + next_j + 1
        faces.append((v0, v1, v2))
        faces.append((v0, v2, v3))

# write to OBJ
with open("donut.obj", "w") as f:
    for v in vertices:
        f.write(f"v {v[0]} {v[1]} {v[2]}\n")
    for face in faces:
        f.write(f"f {face[0]} {face[1]} {face[2]}\n")

print("donut.obj generated!")
