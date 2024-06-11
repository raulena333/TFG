import matplotlib.pyplot as plt

# Datos para cada material
material1 = [
    (1.000E+00, 3.200E-01),
    (1.500E+00, 1.086E-01),
    (1.560E+00, 9.777E-02),
    (1.500E+00, 1.068E+00),
    (2.000E+00, 6.110E-01),
    (3.000E+00, 2.128E-01),
    (4.000E+00, 9.734E-02),
    (5.000E+00, 5.222E-02),
    (6.000E+00, 3.113E-02),
    (8.000E+00, 1.359E-02),
    (1.000E+01, 7.077E-04)
]

material2 = [
    (1.000E+00, 9.090E+00),
    (1.047E+00, 8.026E+00),
    (1.096E+00, 7.091E+00),
    (1.096E+00, 8.037E+00),
    (1.500E+00, 3.799E+00),
    (2.000E+00, 1.852E+00),
    (3.000E+00, 6.440E-01),
    (4.000E+00, 2.987E-01),
    (5.000E+00, 1.633E-01),
    (6.000E+00, 9.942E-02),
    (8.000E+00, 4.519E-02),
    (8.979E+00, 3.293E-02),
    (8.979E+00, 2.393E-01),
    (1.000E+01, 1.858E-01)
]

material3 = [
    (1.000E+00, 4.017E-01),
    (1.050E+00, 1.316E-01),
    (2.000E+00, 5.804E-02),
    (3.000E+00, 1.777E-02),
    (4.000E+00, 7.542E-03),
    (5.000E+00, 3.853E-03),
    (6.000E+00, 2.219E-03),
    (8.000E+00, 9.315E-04),
    (1.000E+01, 4.805E-04)
]

# Extracción de datos para cada material
energias1, atenuaciones1 = zip(*material1)
energias2, atenuaciones2 = zip(*material2)
energias3, atenuaciones3 = zip(*material3)

# Crear el gráfico
plt.semilogy(energias1, atenuaciones1, label='Aluminio', color='red')  # Mylar (rojo)
plt.semilogy(energias2, atenuaciones2, label='Cobre', color='blue') # Aluminio (azul)
plt.semilogy(energias3, atenuaciones3, label='Mylar', color='green') # Cobre (verde)

# Etiquetas y leyenda
plt.xlabel('Energía (keV)', fontsize=14)
plt.ylabel('Atenuación (µm$^{-1}$)', fontsize=14)
plt.legend(fontsize=12)

# Ajustar el tamaño de los números de los ejes
plt.xticks(fontsize=13)
plt.yticks(fontsize=13)

# Ajustar los márgenes
plt.subplots_adjust(left=0.15, bottom=0.15)  # Aumenta el margen izquierdo y el margen inferior

# Guardar el gráfico como un archivo PNG
plt.savefig('atenuacion_materiales.png')
plt.savefig('atenuacion_materiales.pdf')

# Mostrar el gráfico
plt.show()
