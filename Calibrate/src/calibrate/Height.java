package calibrate;

import java.awt.Point;

public class Height {
	
	Point i;
	Point q;
	Point h;
	Point p;
	Point t_r;
	Point t;
	Point b_r;
	Point b;
	Point l;
	
	int[] findVanishingLine(Point point1, Point point2, Point point3, Point point4)
	{
		//first vanishing point of the vanishing line
		int A1 = point2.y - point1.y;
		int A2 = point4.y - point3.y;
		int B1 = point1.x - point2.x;
		int B2 = point3.x - point4.x;
		int C1 = A1*point1.x + B1*point1.y;
		int C2 = A2*point3.x + B1*point3.y;

		int delta = A1*B2 - A2*B1;
		if(delta == 0) 
			throw new ArithmeticException("Lines are parallel");

		int x1 = (B2*C1 - B1*C2)/delta;
		int y1 = (A1*C2 - A2*C1)/delta;

		//second vanishing point of the vanishing line
		A1 = point1.y - point3.y;
		A2 = point2.y - point4.y;
		B1 = point3.x - point1.x;
		B2 = point4.x - point2.x;
		C1 = A1*point3.x + B1*point3.y;
		C2 = A2*point4.x + B1*point4.y;

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

	Point findIntersectionQ(Point b, Point b_r, int[] l)
	{
		int A1 = b_r.y - b.y;
		int A2 = l[0];
		int B1 = b.x - b_r.x;
		int B2 = l[1];
		int C1 = A1*b.x + B1*b.y;
		int C2 = l[2];

		int delta = A1*B2 - A2*B1;
		if(delta == 0) 
			throw new ArithmeticException("Lines are parallel");

		int x = (B2*C1 - B1*C2)/delta;
		int y = (A1*C2 - A2*C1)/delta;

		Point v = new Point();
		v.x = x;
		v.y = y;

		return v;
	}

	Point findIntersectionI(Point q, Point t_r, Point p, Point b)
	{
		int A1 = t_r.y - q.y;
		int A2 = b.y - p.y;
		int B1 = q.x - t_r.x;
		int B2 = p.x - b.x;
		int C1 = A1*q.x + B1*q.y;
		int C2 = A2*p.x + B1*p.y;

		int delta = A1*B2 - A2*B1;
		if(delta == 0) 
			throw new ArithmeticException("Lines are parallel");

		int x = (B2*C1 - B1*C2)/delta;
		int y = (A1*C2 - A2*C1)/delta;

		Point v = new Point();
		v.x = x;
		v.y = y;

		return v;
	}

	Point findVerticalVanishingPoint(Point point1, Point point2, Point point3, Point point4)
	{
		//find vertical vanishing point
		int A1 = point2.y - point1.y;
		int A2 = point4.y - point3.y;
		int B1 = point1.x - point2.x;
		int B2 = point3.x - point4.x;
		int C1 = A1*point1.x + B1*point1.y;
		int C2 = A2*point3.x + B1*point3.y;

		int delta = A1*B2 - A2*B1;
		if(delta == 0) 
			throw new ArithmeticException("Lines are parallel");

		int x = (B2*C1 - B1*C2)/delta;
		int y = (A1*C2 - A2*C1)/delta;

		Point v = new Point();
		v.x = x;
		v.y = y;

		return v;
	}

	int distance(Point p1, Point p2)
	{
		int d = ((p2.x-p1.x)^2+(p2.y-p1.y)^2)^1/2;
		return d;
	}

	int[][] findHomography(Point b, Point p, Point i, int h_r)
	{
		int[][] H = new int[2][2];

		H[0][0] = h_r*(distance(p,b) - distance(i,b));
		H[0][1]= 0;
		H[1][0] = -distance(i,b);
		H[1][1] = distance(p,b)*distance(i,b);

		return H;
	}

	int findHeight(int[][] H, Point b, Point t)
	{
		Point s = new Point();

		Point aux = new Point();
		aux.x = distance(t,b);
		aux.y = 1;

		s.x = H[0][0]*aux.x + H[0][1]*aux.y;
		s.y = H[1][0]*aux.x + H[1][1]*aux.y;

		return s.x/s.y;
	}
}
