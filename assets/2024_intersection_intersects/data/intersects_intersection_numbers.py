from fractions import Fraction
from decimal import Decimal

def orient2d(p1, p2, p3):
    """
    Calcule le double de l'aire du triangle formé par les points p1, p2 et p3.
    Si le résultat est positif, les points sont orientés dans le sens antihoraire.
    Si le résultat est négatif, les points sont orientés dans le sens horaire.
    Si le résultat est nul, les points sont alignés.
    """
    return (p2.x - p1.x) * (p3.y - p1.y) - (p2.y - p1.y) * (p3.x - p1.x)

def intersection_segments(seg1, seg2):
    # Coordonnées des segments
    p1, p2 = seg1
    p3, p4 = seg2
    
    # Calcul des orientations
    orient1 = orient2d(p1, p2, p3)
    orient2 = orient2d(p1, p2, p4)
    orient3 = orient2d(p3, p4, p1)
    orient4 = orient2d(p3, p4, p2)
    
    # Vérifier si les segments peuvent potentiellement s'intersecter
    if orient1 * orient2 <= 0 and orient3 * orient4 <= 0:
        # Calcul des coefficients des équations des droites
        m1 = (p2.y - p1.y) / (p2.x - p1.x) if (p2.x - p1.x) != 0 else float('inf')
        m2 = (p4.y - p3.y) / (p4.x - p3.x) if (p4.x - p3.x) != 0 else float('inf')
        b1 = p1.y - m1 * p1.x if m1 != float('inf') else p1.x
        b2 = p3.y - m2 * p3.x if m2 != float('inf') else p3.x
        
        # Calcul du point d'intersection
        if m1 == m2:
            # Segments parallèles
            return None
        elif m1 == float('inf'):
            # Droite 1 verticale
            x_intersect = p1.x
            y_intersect = m2 * x_intersect + b2
        elif m2 == float('inf'):
            # Droite 2 verticale
            x_intersect = p3.x
            y_intersect = m1 * x_intersect + b1
        else:
            # Calcul du point d'intersection pour des droites non verticales
            x_intersect = (b2 - b1) / (m1 - m2)
            y_intersect = m1 * x_intersect + b1
        
        # Vérifier si le point d'intersection est sur les segments
        if min(p1.x, p2.x) <= x_intersect <= max(p1.x, p2.x) and min(p3.x, p4.x) <= x_intersect <= max(p3.x, p4.x) and min(p1.y, p2.y) <= y_intersect <= max(p1.y, p2.y) and min(p3.y, p4.y) <= y_intersect <= max(p3.y, p4.y):
            return MyPoint(x_intersect, y_intersect)
        else:
            return None  # Intersection en dehors des segments
    else:
        return None  # Pas d'intersection

class MyPoint:
    def __init__(self, x: float, y: float):
        self.x = x
        self.y = y

    def __repr__(self):
        return f"Point({self.x}, {self.y})"

# Utilisation des nombres floatants
print("Float")
ptA = MyPoint(1981816.95535941, 5199128.88909491)
ptB = MyPoint(1981750.03865557, 5199364.0407664)
ptC = MyPoint(1981712.74128724, 5199234.3772356)
ptD = MyPoint(1981867.15655533, 5199279.89107609)
intersection = intersection_segments((ptA, ptB), (ptC, ptD))
print(f"Point({float(intersection.x):.17f} {float(intersection.y):.17f})")
print(orient2d(ptA, ptB, intersection))

# Avec Decimal
print("Decimal")
ptA_d = MyPoint(Decimal(ptA.x), Decimal(ptA.y))
ptB_d = MyPoint(Decimal(ptB.x), Decimal(ptB.y))
ptC_d = MyPoint(Decimal(ptC.x), Decimal(ptC.y))
ptD_d = MyPoint(Decimal(ptD.x), Decimal(ptD.y))
intersection = intersection_segments((ptA_d, ptB_d), (ptC_d, ptD_d))
print(f"Point({float(intersection.x):.17f} {float(intersection.y):.17f})")
print(float(orient2d(ptA_d, ptB_d, intersection)))

# Avec Fraction
print("Fraction")
ptA_f = MyPoint(Fraction(ptA.x), Fraction(ptA.y))
ptB_f = MyPoint(Fraction(ptB.x), Fraction(ptB.y))
ptC_f = MyPoint(Fraction(ptC.x), Fraction(ptC.y))
ptD_f = MyPoint(Fraction(ptD.x), Fraction(ptD.y))
intersection = intersection_segments((ptA_f, ptB_f), (ptC_f, ptD_f))
print(f"Point({float(intersection.x):.17f} {float(intersection.y):.17f})")
print(float(orient2d(ptA_f, ptB_f, intersection)))
