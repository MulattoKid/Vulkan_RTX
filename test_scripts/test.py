import numpy as np

# Using the above nested radical formula for g=phi_d 
# or you could just hard-code it. 
# phi(1) = 1.61803398874989484820458683436563 
# phi(2) = 1.32471795724474602596090885447809 
def phi(d): 
  x=2.0000 
  for i in range(10): 
    x = pow(1.0+x,1.0/(d+1.0)) 
  return x

# Number of dimensions. 
d=2 

# number of required points 
n=64

g = phi(d)
print(g) 
alpha = np.zeros(d) 
for j in range(d): 
  alpha[j] = pow(1.0/g,j+1.0) %1 
z = np.zeros((n, d)) 
print(alpha)

# This number can be any real number. 
# Common default setting is typically seed=0
# But seed = 0.5 is generally better. 
for i in range(n): 
  z[i] = (0.5 + alpha*(i+1.0)) %1
  print(i, z[i][0], z[i][1])
