import numpy as np
from numpy.polynomial.polynomial import Polynomial

# Given points (cubeSizeHalf, cubeSizeLimit)
xy_pairs = np.array([
    (0.25, 0.5),
    (0.3125, 0.532),
    (0.375, 0.554),
    (0.4375, 0.578)
])

# Extract x and y values
x_values = xy_pairs[:, 0]
y_values = xy_pairs[:, 1]

# Fit a quadratic polynomial (degree 2)
coefficients = np.polyfit(x_values, y_values, 2)

# Create the polynomial equation
quadratic_poly = Polynomial(coefficients[::-1])

a, b, c = [round(val, 4) for val in coefficients]
print("The quadratic equation is:")
print(f"y = {a}x^2 + {b}x + {c}")
print("")
for val in xy_pairs:
    corrected_y = a*val[0]*val[0] + b*val[0] + c
    print(f"x={val[0]}; original_y={val[1]}; corrected_y={corrected_y}")