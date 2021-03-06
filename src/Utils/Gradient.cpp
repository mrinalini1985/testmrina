// TortoiseSVN - a Windows shell extension for easy version control

// Copyright (C) 2003 - Tim Kemp and Stefan Kueng

// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

#include "StdAfx.h"
#include "gradient.h"

CGradient::CGradient(void)
{
}

CGradient::~CGradient(void)
{
}

void CGradient::Draw(CDC * pDC, CRect rect, COLORREF colorStart, COLORREF colorEnd, BOOL bHorz/* = TRUE*/, UINT nSteps/* = 64*/)
{
    for (UINT i = 0; i < nSteps; i++)
    {
        BYTE bR = (BYTE) ((GetRValue(colorStart) * (nSteps - i) + GetRValue(colorEnd) * i) / nSteps);
        BYTE bG = (BYTE) ((GetGValue(colorStart) * (nSteps - i) + GetGValue(colorEnd) * i) / nSteps);
        BYTE bB = (BYTE) ((GetBValue(colorStart) * (nSteps - i) + GetBValue(colorEnd) * i) / nSteps);

		CBrush br (RGB(bR, bG, bB));

        CRect r2 = rect;
        if (!bHorz)
        {
            r2.top = rect.top + ((i * rect.Height()) / nSteps);
            r2.bottom = rect.top + (((i + 1) * rect.Height()) / nSteps);
            if (r2.Height() > 0)
                pDC->FillRect(r2, &br);
        }
        else
        {
            r2.left = rect.left + ((i * rect.Width()) / nSteps);
            r2.right = rect.left + (((i + 1) * rect.Width()) / nSteps);
            if (r2.Width() > 0)
                pDC->FillRect(r2, &br);
        }
    } // for (int i = 0; i < nSteps; i++)
}

void CGradient::Draw(CDC * pDC, CRect rect, COLORREF colorStart, COLORREF colorMid, COLORREF colorEnd, BOOL bHorz/* = TRUE*/, UINT nSteps/* = 64*/)
{
	CRect rect1 = rect;
	CRect rect2 = rect;

	if (!bHorz)
	{
		rect1.top = (rect.bottom + rect.top) / 2;
		rect2.bottom = (rect.bottom + rect.top) / 2;
	}
	else
	{
		rect1.right = (rect.left + rect.right) / 2;
		rect2.left = (rect.left + rect.right) / 2;
	}

	Draw(pDC, rect1, colorStart, colorMid, bHorz, nSteps/2);
	Draw(pDC, rect2, colorMid, colorEnd, bHorz, nSteps/2);
}

#ifdef USE_GDI_GRADIENT

void CGradient::DrawGDI(CDC * pDC, CRect rect, COLORREF colorStart, COLORREF colorEnd, BOOL bHorz/* = TRUE*/)
{
	TRIVERTEX        vert[2] ;
	GRADIENT_RECT    gRect;
	vert [0] .x      = rect.left;
	vert [0] .y      = rect.top;
	vert [0] .Red    = GetRValue(colorStart)<<8;
	vert [0] .Green  = GetGValue(colorStart)<<8;
	vert [0] .Blue   = GetBValue(colorStart)<<8;
	vert [0] .Alpha  = 0x0000;

	vert [1] .x      = rect.right;
	vert [1] .y      = rect.bottom; 
	vert [1] .Red    = GetRValue(colorEnd)<<8;
	vert [1] .Green  = GetGValue(colorEnd)<<8;
	vert [1] .Blue   = GetBValue(colorEnd)<<8;
	vert [1] .Alpha  = 0x0000;

	gRect.UpperLeft  = 0;
	gRect.LowerRight = 1;
	if (bHorz)
		pDC->GradientFill(vert, 2, &gRect, 1, GRADIENT_FILL_RECT_H);
	else
		pDC->GradientFill(vert, 2, &gRect, 1, GRADIENT_FILL_RECT_V);

}

void CGradient::DrawGDI(CDC * pDC, CRect rect, COLORREF colorStart, COLORREF colorMid, COLORREF colorEnd, BOOL bHorz/* = TRUE*/)
{
	CRect rect1 = rect;
	CRect rect2 = rect;

	if (!bHorz)
	{
		rect1.top = (rect.bottom + rect.top) / 2;
		rect2.bottom = (rect.bottom + rect.top) / 2;
	}
	else
	{
		rect1.right = (rect.left + rect.right) / 2;
		rect2.left = (rect.left + rect.right) / 2;
	}

	DrawGDI(pDC, rect1, colorStart, colorMid, bHorz);
	DrawGDI(pDC, rect2, colorMid, colorEnd, bHorz);
}

#endif