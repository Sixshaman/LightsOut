/*
Copyright (c) 2015 Sixshaman

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
documentation files (the "Software"), to deal in the Software without restriction, including without limitation the
rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and
to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

/*
 * With that shader drawing is much faster
 */

cbuffer cbParams: register(b0)
{
	uint gFieldSize;
	uint gCellSize;
	bool gSolveVisible;
	uint gCompressedTurn; //Coordinates of hint

	float4 gColorNone;
	float4 gColorEnabled;
	float4 gColorSolved;
	float4 gColorBetween;
};

Texture1D<uint> Field: register(t0); //Field in compressed UINT-format
Texture1D<uint> Solve: register(t1); //Solution in compressed UINT-format

RWTexture2D<float4> Result: register(u0); //Drawn field

[numthreads(16, 16, 1)]
void main(uint3 DTid: SV_DispatchThreadID)
{
	float4 result = float4(0.0f, 0.0f, 0.0f, 0.0f);

	[flatten]
	if((DTid.x % gCellSize) && (DTid.y % gCellSize)) //Inside the cell
	{
		uint2 cellNumber = DTid.xy / gCellSize.xx;

		uint cellNumberAll = cellNumber.y * gFieldSize + cellNumber.x; //Number of cell

		uint compressedCellGroupNumber = cellNumberAll / 32; //Element of Field that contains that cell
		uint compressedCellNumber      = cellNumberAll % 32; //Number of bit of that cell

		bool cellEnabled = (Field[compressedCellGroupNumber] >> compressedCellNumber) & 1; //Getting the bit of cell

		[flatten]
		if(cellEnabled)
		{
			result = gColorEnabled;
		}
		else
		{
			result = gColorNone;
		}

		[flatten]
		if(gSolveVisible) //We are showing the solution
		{
			bool cellSolved = (Solve[compressedCellGroupNumber] >> compressedCellNumber) & 1; //Getting the bit of cell

			[flatten]
			if(cellSolved)
			{
				result = gColorSolved;
			}
		}
		else //We are not showing the solution
		{
			uint hintTurnX = gCompressedTurn >> 16;
			uint hintTurnY = gCompressedTurn & 0xffff;

			[flatten]
			if(cellNumber.x == hintTurnX && cellNumber.x == hintTurnY) //This cell is a hint cell
			{
				result = gColorSolved;
			}
		}
	}
	else //Edge of the cell
	{
		result = gColorBetween;
	}

	Result[DTid.xy] = result;
}