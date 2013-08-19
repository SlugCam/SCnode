package calibrate;

public class Height {
	
	int[] i;
	int[] q;
	int[] h;
	int[] p;
	int[] t_r;
	int[] t;
	int[] b_r;
	int[] b;
	int[] l;
	
	int[] findVanishingLine(int[] point1, int[] point2, int[] point3, int[] point4)
	{
		//first vanishing point of the vanishing line
		int A1 = point2[1] - point1[1];
		int A2 = point4[1] - point3[1];
		int B1 = point1[0] - point2[0];
		int B2 = point3[0] - point4[0];
		int C1 = A1*point1[0] + B1*point1[1];
		int C2 = A2*point3[0] + B1*point3[1];

		int delta = A1*B2 - A2*B1;
		if(delta == 0) 
			throw new ArithmeticException("Lines are parallel");

		int x1 = (B2*C1 - B1*C2)/delta;
		int y1 = (A1*C2 - A2*C1)/delta;

		//second vanishing point of the vanishing line
		A1 = point1[1] - point3[1];
		A2 = point2[1] - point4[1];
		B1 = point3[0] - point1[0];
		B2 = point4[0] - point2[0];
		C1 = A1*point3[0] + B1*point3[1];
		C2 = A2*point4[0] + B1*point4[1];

		delta = A1*B2 - A2*B1;
		if(delta == 0)
			throw new ArithmeticException("Lines are parallel");
		int x2 = (B2*C1 - B1*C2)/delta;
		int y2 = (A1*C2 - A2*C1)/delta;

		//find the vanishing line
		int Av = y2 - y1;
		int Bv = x2 - x1;
		int Cv = Av*x1 + Bv*y1;

		int[] v = new int[3];
		v[0] = Av;
		v[1] = Bv;
		v[2] = Cv;

		return v;
	}

	int[] findIntersectionQ(int[] b, int[] b_r, int[] l)
	{
		int A1 = b_r[1] - b[1];
		int A2 = l[0];
		int B1 = b[0] - b_r[0];
		int B2 = l[1];
		int C1 = A1*b[0] + B1*b[1];
		int C2 = l[2];

		int delta = A1*B2 - A2*B1;
		if(delta == 0) 
			throw new ArithmeticException("Lines are parallel");

		int x = (B2*C1 - B1*C2)/delta;
		int y = (A1*C2 - A2*C1)/delta;

		int[] v = new int[2];
		v[0] = x;
		v[1] = y;

		return v;
	}

	int[] findIntersectionI(int[] q, int[] t_r, int[] p, int[] b)
	{
		int A1 = t_r[1] - q[1];
		int A2 = b[1] - p[1];
		int B1 = q[0] - t_r[0];
		int B2 = p[0] - b[0];
		int C1 = A1*q[0] + B1*q[1];
		int C2 = A2*p[0] + B1*p[1];

		int delta = A1*B2 - A2*B1;
		if(delta == 0) 
			throw new ArithmeticException("Lines are parallel");

		int x = (B2*C1 - B1*C2)/delta;
		int y = (A1*C2 - A2*C1)/delta;

		int[] v = new int[2];
		v[0] = x;
		v[1] = y;

		return v;
	}

	int[] findVerticalVanishingPoint(int[] point1, int[] point2, int[] point3, int[] point4)
	{
		//find vertical vanishing point
		int A1 = point2[1] - point1[1];
		int A2 = point4[1] - point3[1];
		int B1 = point1[0] - point2[0];
		int B2 = point3[0] - point4[0];
		int C1 = A1*point1[0] + B1*point1[1];
		int C2 = A2*point3[0] + B1*point3[1];

		int delta = A1*B2 - A2*B1;
		if(delta == 0) 
			throw new ArithmeticException("Lines are parallel");

		int x = (B2*C1 - B1*C2)/delta;
		int y = (A1*C2 - A2*C1)/delta;

		int[] v = new int[2];
		v[0] = x;
		v[1] = y;

		return v;
	}

	int distance(int[] p1, int[] p2)
	{
		int d = ((p2[0]-p1[0])^2+(p2[1]-p1[1])^2)^1/2;
		return d;
	}

	int[][] findHomography(int[] b, int[] p, int[] i, int h_r)
	{
		int[][] H = new int[2][2];

		H[0][0] = h_r*(distance(p,b) - distance(i,b));
		H[0][1] = 0;
		H[1][0] = -distance(i,b);
		H[1][1] = distance(p,b)*distance(i,b);

		return H;
	}

	int findHeight(int[][] H, int[] b, int[] t)
	{
		int[] s = new int[2];

		int[] aux = new int[2];
		aux[0] = distance(t,b);
		aux[1] = 1;

		s[0] = H[0][0]*aux[0] + H[0][1]*aux[1];
		s[1] = H[1][0]*aux[0] + H[1][1]*aux[1];

		return s[0]/s[1];
	}
}
