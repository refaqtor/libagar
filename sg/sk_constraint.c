/*
 * Copyright (c) 2007 Hypertriton, Inc. <http://hypertriton.com/>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 * USE OF THIS SOFTWARE EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 * These functions implement placement of entities based on their constraints
 * with respect to either single entity or a set of two constrained entities.
 *
 * They are implemented as generic sketch instructions (which are usually
 * going to be generated by DOF analysis).
 *
 * Where there are multiple solutions, we optimize for least displacement
 * of the unknown entity.
 */ 

#include <config/have_opengl.h>
#ifdef HAVE_OPENGL

#include <core/core.h>

#include "sk.h"
#include "sk_constraint.h"

static int
PtFromPtAtDistance(SK_Constraint *ct, void *self, void *other)
{
	SG_Vector p1 = SK_Pos(self);
	SG_Vector p2 = SK_Pos(other);
	SG_Real theta;
	
	VecVecAngle(p1, p2, &theta, NULL);
	SK_Identity(self);
	p2.x -= ct->ct_distance*Cos(theta);
	p2.y -= ct->ct_distance*Sin(theta);
	SK_Translatev(self, &p2);
	return (0);
}

static int
PtFromLineAtDistance(SK_Constraint *ct, void *self, void *other)
{
	SK_Line *L = other;
	SG_Vector pOrig = SK_Pos(self);
	SG_Vector p1 = SK_Pos(L->p1);
	SG_Vector p2 = SK_Pos(L->p2);
	SG_Vector v, vd, s1, s2;
	SG_Real mag, u, theta;

	/* Find the closest point on the line. */
	vd = VecSubp(&p2, &p1);
	mag = VecDistance(p2, p1);
	u = ( ((pOrig.x - p1.x)*(p2.x - p1.x)) +
	      ((pOrig.y - p1.y)*(p2.y - p1.y)) ) / (mag*mag);
	
	v = VecAdd(p1, VecScalep(&vd,u));
	if (ct->ct_distance == 0.0) {
		SK_Identity(self);
		SK_Translatev(self, &v);
		return (0);
	}
	VecVecAngle(p1, p2, &theta, NULL);
	theta += M_PI/2.0;
	s1.x = v.x + ct->ct_distance*Cos(theta);
	s1.y = v.y + ct->ct_distance*Sin(theta);
	s2.x = v.x - ct->ct_distance*Cos(theta);
	s2.y = v.y - ct->ct_distance*Sin(theta);
	
	SK_Identity(self);
	if (VecDistancep(&pOrig,&s1) < VecDistancep(&pOrig,&s2)) {
		SK_Translatev(self, &s1);
	} else {
		SK_Translatev(self, &s2);
	}
	return (0);
}

#if 0
static int
PtFromDistantCircle(SK_Constraint *ct, void *self, void *other)
{
	SK_Point *p = self;
	SK_Circle *C1 = other;
	SG_Vector p1 = SK_Pos(C1->p);
	
	SK_Identity(p);
	SK_Translatev(p, &p1);
	SK_Translate2(p, C1->r + ct->ct_distance, 0.0);
	return (0);
}
#endif

static int
LineFromLineAtDistance(SK_Constraint *ct, void *self, void *other)
{
	SK_Line *L = self;
	SK_Line *L1 = other;

	SK_MatrixCopy(L->p1, L1->p1);
	SK_MatrixCopy(L->p2, L1->p2);
	SK_Translate2(L->p1, ct->ct_distance, 0.0);
	SK_Translate2(L->p2, ct->ct_distance, 0.0);
	return (0);
}

static int
LineFromLineAtAngle(SK_Constraint *ct, void *pSelf, void *pFixed)
{
	SK_Line *self = pSelf;
	SK_Line *fixed = pFixed;
	SG_Vector vShd, vSelf, vFixed, v;
	SG_Real len, theta;
	SK_Point *p;

	theta = ct->ct_angle;

	if (self->p1 == fixed->p1 || self->p1 == fixed->p2) {
		vShd = SK_Pos(self->p1);
		vSelf = SK_Pos(self->p2);
		vFixed = (self->p1==fixed->p1) ? SK_Pos(fixed->p2) :
		                                 SK_Pos(fixed->p1);
		p = self->p2;
//		theta = -ct->ct_angle;
	} else if (self->p2 == fixed->p1 || self->p2 == fixed->p2) {
		vShd = SK_Pos(self->p2);
		vSelf = SK_Pos(self->p1);
		vFixed = (self->p2==fixed->p1) ? SK_Pos(fixed->p2) :
		                                 SK_Pos(fixed->p1);
		p = self->p1;
//		theta = ct->ct_angle;
	} else {
		AG_SetError("No shared point between lines");
		return (-1);
	}
	VecSubv(&vSelf, &vShd);
	VecSubv(&vFixed, &vShd);
	len = VecLenp(&vSelf);
	VecNormv(&vFixed);
	v = VecSub(vFixed, VecNorm(vSelf));
	SK_Identity(p);
	SK_Translate2(p,
	    vShd.x + Cos(theta)*vFixed.x*len - Sin(theta)*vFixed.y*len,
	    vShd.y + Sin(theta)*vFixed.x*len + Cos(theta)*vFixed.y*len);
	return (0);
}

/*
 * Compute the position of a point relative to two fixed points from
 * distance/incidence constraints. This is a system of two quadratic
 * equations describing the intersection of two circles.
 */
static int
PtFromPtPt(void *self, SK_Constraint *ct1, void *n1, SK_Constraint *ct2,
    void *n2)
{
	SK *sk = SKNODE(self)->sk;
	SG_Vector pOrig = SK_Pos(self);
	SG_Vector p1 = SK_Pos(n1);
	SG_Vector p2 = SK_Pos(n2);
	SG_Real d1 = (ct1->type == SK_DISTANCE) ? ct1->ct_distance : 0.0;
	SG_Real d2 = (ct2->type == SK_DISTANCE) ? ct2->ct_distance : 0.0;
	SG_Real d12 = VecDistancep(&p1,&p2);
	SG_Real a, h, b;
	SG_Vector p, s1, s2;

	printf("PtFromPtPt(%s,[%f:%s],[%f:%s])\n",
	    SKNODE(self)->name,
	    ct1->ct_distance, SKNODE(n1)->name,
	    ct2->ct_distance, SKNODE(n2)->name);

	if (d12 > (d1+d2)) {
		AG_SetError("%s and %s are too far apart to satisfy "
		            "constraint: %.02f > (%.02f+%.02f)",
			    SKNODE(n1)->name, SKNODE(n2)->name, d12, d1, d2);
		return (-1);
	}
	if (d12 < SG_Fabs(d1-d2)) {
		AG_SetError("%s and %s are too close to satisfy "
		            "constraint: %.02f < |%.02f-%.02f|",
			    SKNODE(n1)->name, SKNODE(n2)->name, d12, d1, d2);
		return (-1);
	}

	a = (d1*d1 - d2*d2 + d12*d12) / (2.0*d12);
	h = Sqrt(d1*d1 - a*a);
	p = VecLERPp(&p1, &p2, a/d12);
	b = h/d12;

	s1.x = p.x - b*(p2.y - p1.y);
	s1.y = p.y + b*(p2.x - p1.x);
	s1.z = 0.0;
	s2.x = p.x + b*(p2.y - p1.y);
	s2.y = p.y - b*(p2.x - p1.x);
	s2.z = 0.0;

	SK_Identity(self);
	if (VecDistancep(&s1,&s2) == 0.0) {
		SK_Translatev(self, &s1);
		sk->nSolutions++;
	} else {
		if (VecDistancep(&pOrig,&s1) < VecDistancep(&pOrig,&s2)) {
			SK_Translatev(self, &s1);
		} else {
			SK_Translatev(self, &s2);
		}
		sk->nSolutions+=2;
	}
	return (0);
}

/*
 * Compute the position of a point relative to a known point and line.
 * This is a system of quadratic equations describing the intersection of
 * a line with a circle.
 */
static int
PtFromPtLine(void *self, SK_Constraint *ct1, void *n1,
    SK_Constraint *ct2, void *n2)
{
	SK *sk = SKNODE(self)->sk;
	SG_Vector pOrig = SK_Pos(self);
	SG_Vector p = SK_Pos(n1);
	SK_Line *L = n2;
	SG_Vector p1 = SK_Pos(L->p1);
	SG_Vector p2 = SK_Pos(L->p2);
	SG_Real d1 = (ct1->type == SK_DISTANCE) ? ct1->ct_distance : 0.0;
	SG_Real a = (p2.x - p1.x)*(p2.x - p1.x) +
	            (p2.y - p1.y)*(p2.y - p1.y);
	SG_Real b = 2.0*( (p2.x - p1.x)*(p1.x - p.x) +
	                  (p2.y - p1.y)*(p1.y - p.y));
	SG_Real cc = p.x*p.x + p.y*p.y + p1.x*p1.x + p1.y*p1.y -
	             2.0*(p.x*p1.x + p.y*p1.y) - d1*d1;
	SG_Real deter = b*b - 4*a*cc;
	SG_Vector s[2];
	int nSolutions = 0;

	if (deter < 0.0) {
		printf("outside (det < 0)\n");
		goto fail;
	} else if (deter == 0.0) {
		/* TODO Tangent */
		printf("tangent (det==0)\n");
	} else {
		SG_Real e = Sqrt(deter);
		SG_Real u1 = (-b + e) / (2.0*a);
		SG_Real u2 = (-b - e) / (2.0*a);
		
		printf("e=%f, u1=%f, u2=%f\n", e, u1, u2);
		
		if ((u1 < 0.0 || u1 > 1.0) &&
		    (u2 < 0.0 || u2 > 1.0)) {
			if ((u1 < 0.0 && u2 < 0.0) ||
			    (u1 > 1.0 && u2 > 1.0)) {
				goto fail;
			} else {
				printf("u1=%f, u2=%f!\n", u1, u2);
			}
		} else {
			if (u1 >= 0.0 && u1 <= 1.0)
				s[nSolutions++] = VecLERPp(&p1,&p2,u1);
			if (u2 >= 0.0 && u2 <= 1.0)
				s[nSolutions++] = VecLERPp(&p1,&p2,u1);
		}
	}
	
	SK_Identity(self);
	if (nSolutions == 2) {
		if (VecDistancep(&pOrig,&s[0]) < VecDistancep(&pOrig,&s[1])) {
			SK_Translatev(self, &s[0]);
		} else {
			SK_Translatev(self, &s[1]);
		}
	} else if (nSolutions == 1) {
		SK_Translatev(self, &s[0]);
	} else {
		printf("no solutions\n");
		goto fail;
	}
	sk->nSolutions += nSolutions;
	return (0);
fail:
	AG_SetError("%s and %s are too far apart to satisfy constraint",
		    SKNODE(n1)->name, SKNODE(n2)->name);
	return (-1);
}

/*
 * Compute the position of a point p from two known lines L1 and L2.
 * This is a system of two linear equations describing the intersection
 * of two imaginary lines at distances d1 and d2 from L1 and L2.
 */
static int
PtFromLineLine(void *self, SK_Constraint *ct1, void *n1,
    SK_Constraint *ct2, void *n2)
{
#if 0
	SK *sk = SKNODE(self)->sk;
	SG_Vector pOrig = SK_Pos(self);
	SK_Line *L1 = n1;
	SK_Line *L2 = n2;
	int nSolutions = 0;
#endif
	/* TODO */
	AG_SetError("not yet");
	return (-1);
}

const SK_ConstraintPairFn skConstraintPairFns[] = {
	{ SK_DISTANCE,	 "Point:*", "Point:*",	PtFromPtAtDistance },
	{ SK_DISTANCE,	 "Point:*", "Line:*",	PtFromLineAtDistance },
	{ SK_DISTANCE,	 "Line:*",  "Line:*",	LineFromLineAtDistance },
	{ SK_ANGLE,	 "Line:*",  "Line:*",	LineFromLineAtAngle },
};
const SK_ConstraintRingFn skConstraintRingFns[] = {
	{
		"Point:*",
		SK_CONSTRAINT_ANY, "Point:*",
		SK_CONSTRAINT_ANY, "Point:*",
		PtFromPtPt
	}, {
		"Point:*",
		SK_CONSTRAINT_ANY, "Point:*",
		SK_CONSTRAINT_ANY, "Line:*",
		PtFromPtLine
	}, {
		"Point:*",
		SK_CONSTRAINT_ANY, "Line:*",
		SK_CONSTRAINT_ANY, "Line:*",
		PtFromLineLine
	},
};

const int skConstraintPairFnCount = sizeof(skConstraintPairFns) /
                                    sizeof(skConstraintPairFns[0]);
const int skConstraintRingFnCount = sizeof(skConstraintRingFns) /
                                    sizeof(skConstraintRingFns[0]);

#endif /* HAVE_OPENGL */
