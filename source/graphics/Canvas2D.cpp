/* Copyright (C) 2021 Wildfire Games.
 * This file is part of 0 A.D.
 *
 * 0 A.D. is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * 0 A.D. is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with 0 A.D.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "precompiled.h"

#include "Canvas2D.h"

#include "graphics/Color.h"
#include "graphics/ShaderManager.h"
#include "graphics/TextureManager.h"
#include "gui/GUIMatrix.h"
#include "maths/Rect.h"
#include "maths/Vector2D.h"
#include "ps/CStrInternStatic.h"
#include "renderer/Renderer.h"

#include <array>

void CCanvas2D::DrawLine(const std::vector<CVector2D>& points, const float width, const CColor& color)
{
	std::vector<float> vertices;
	vertices.reserve(points.size() * 3);
	for (const CVector2D& point : points)
	{
		vertices.emplace_back(point.X);
		vertices.emplace_back(point.Y);
		vertices.emplace_back(0.0f);
	}

	// Setup the render state
	CMatrix3D transform = GetDefaultGuiMatrix();
	CShaderDefines lineDefines;
	CShaderTechniquePtr tech = g_Renderer.GetShaderManager().LoadEffect(str_gui_solid, g_Renderer.GetSystemShaderDefines(), lineDefines);
	tech->BeginPass();
	CShaderProgramPtr shader = tech->GetShader();

	shader->Uniform(str_transform, transform);
	shader->Uniform(str_color, color);
	shader->VertexPointer(3, GL_FLOAT, 0, vertices.data());
	shader->AssertPointersBound();

#if !CONFIG2_GLES
	glEnable(GL_LINE_SMOOTH);
#endif
	glLineWidth(width);
	if (!g_Renderer.DoSkipSubmit())
		glDrawArrays(GL_LINE_STRIP, 0, vertices.size() / 3);
	glLineWidth(1.0f);
#if !CONFIG2_GLES
	glDisable(GL_LINE_SMOOTH);
#endif

	tech->EndPass();
}

void CCanvas2D::DrawRect(const CRect& rect, const CColor& color)
{
	const std::array<float, 8> uvs = {
		0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f
	};
	const std::array<float, 8> vertices = {
		rect.left, rect.bottom,
		rect.right, rect.bottom,
		rect.right, rect.top,
		rect.left, rect.top
	};

	CShaderDefines defines;
	CShaderTechniquePtr tech = g_Renderer.GetShaderManager().LoadEffect(
		str_canvas2d, g_Renderer.GetSystemShaderDefines(), defines);
	tech->BeginPass();
	CShaderProgramPtr shader = tech->GetShader();

	shader->BindTexture(str_tex, g_Renderer.GetTextureManager().GetTransparentTexture());
	shader->Uniform(str_transform, GetDefaultGuiMatrix());
	shader->Uniform(str_colorAdd, color);
	shader->Uniform(str_colorMul, CColor(0.0f, 0.0f, 0.0f, 0.0f));
	shader->VertexPointer(2, GL_FLOAT, 0, vertices.data());
	shader->TexCoordPointer(GL_TEXTURE0, 2, GL_FLOAT, 0, uvs.data());
	shader->AssertPointersBound();

	if (!g_Renderer.DoSkipSubmit())
		glDrawArrays(GL_TRIANGLE_FAN, 0, vertices.size() / 2);

	tech->EndPass();
}
