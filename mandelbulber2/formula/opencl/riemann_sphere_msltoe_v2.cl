/**
 * Mandelbulber v2, a 3D fractal generator  _%}}i*<.        ____                _______
 * Copyright (C) 2018 Mandelbulber Team   _>]|=||i=i<,     / __ \___  ___ ___  / ___/ /
 *                                        \><||i|=>>%)    / /_/ / _ \/ -_) _ \/ /__/ /__
 * This file is part of Mandelbulber.     )<=i=]=|=i<>    \____/ .__/\__/_//_/\___/____/
 * The project is licensed under GPLv3,   -<>>=|><|||`        /_/
 * see also COPYING file in this folder.    ~+{i%+++
 *
 * RiemannSphereMsltoe Version2
 * @reference
 * http://www.fractalforums.com/theory/alternate-co-ordinate-systems/msg11688/#msg11688

 * This file has been autogenerated by tools/populateUiInformation.php
 * from the function "RiemannSphereMsltoeV2Iteration" in the file fractal_formulas.cpp
 * D O    N O T    E D I T    T H I S    F I L E !
 */

REAL4 RiemannSphereMsltoeV2Iteration(REAL4 z, __constant sFractalCl *fractal, sExtendedAuxCl *aux)
{
	REAL theta = 0.0f;
	REAL phi = 0.0f;
	REAL rx;
	REAL rz;
	REAL r = aux->r;

	// rotate
	if (fractal->transformCommon.rotationEnabled)
		z = Matrix33MulFloat4(fractal->transformCommon.rotationMatrix, z);
	// invert and scale
	z *= native_divide(fractal->transformCommon.scale08, r);
	aux->DE = aux->DE * native_divide(fabs(fractal->transformCommon.scale08), r) + 1.0f; //  /r
	// if (fabs(z.x) < 1e-21f) z.x = 1e-21f;
	// if (fabs(z.z) < 1e-21f) z.z = 1e-21f;

	rx = native_divide(z.x, (z.y - 1.0f));
	theta = 8.0f * atan2(2.0f * rx, rx * rx - 1.0f);
	rz = native_divide(z.z, (z.y - 1.0f));
	phi = 8.0f * atan2(2.0f * rz, rz * rz - 1.0f);

	theta *= fractal->transformCommon.scaleA1;
	phi *= fractal->transformCommon.scaleB1;

	rx = native_divide(native_sin(theta), (1.0f + native_cos(theta)));
	rz = native_divide(native_sin(phi), (1.0f + native_cos(phi)));
	REAL rXZ = mad(rx, rx, rz * rz);
	REAL d = native_divide(2.0f, (rXZ + 1.0f));

	REAL a1 = rx * d;
	REAL b1 = (rXZ - 1.0f) * 0.5f * d;
	REAL c1 = rz * d;

	REAL rrrr = r * r * r * r;

	z.x = a1 * rrrr;
	z.y = b1 * rrrr;
	z.z = c1 * rrrr;

	z += fractal->transformCommon.offset010;

	if (fractal->analyticDE.enabled)
	{
		aux->DE = aux->DE * 8.0f * fractal->analyticDE.scale1 * native_divide(length(z), r)
							+ fractal->analyticDE.offset1;
	}
	return z;
}