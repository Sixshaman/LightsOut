#define FLAG_SHOW_SOLUTION  0x01
#define FLAG_SHOW_STABILITY 0x02

cbuffer cbParams: register(b0)
{
	uint gBoardSize;
	uint gCellSize;
	uint gDomainSize;
	uint gFlags;

	float4 gColorNone;
	float4 gColorEnabled;
	float4 gColorSolved;
	float4 gColorBetween;
};

Buffer<uint> Board:     register(t0); //Board (uncompressed)
Buffer<uint> Solution:  register(t1); //Solution(unompressed)
Buffer<uint> Stability: register(t2); //Stability (uncompressed)

RWTexture2D<float4> Result: register(u0); //Drawn Board

uint CellValue(uint2 cellNumber)
{
	uint cellNumberAll = cellNumber.y * gBoardSize + cellNumber.x; //Number of cell
	return Board[cellNumberAll];
}

uint CellValueSolution(uint2 cellNumber)
{
	uint cellNumberAll = cellNumber.y * gBoardSize + cellNumber.x; //Number of cell
	return Solution[cellNumberAll];
}

uint CellValueStability(uint2 cellNumber)
{
	uint cellNumberAll = cellNumber.y * gBoardSize + cellNumber.x; //Number of cell
	return Stability[cellNumberAll];
}

[numthreads(16, 16, 1)]
void main(uint3 DTid: SV_DispatchThreadID)
{
	float4 result = float4(0.0f, 0.0f, 0.0f, 0.0f);

	[flatten]
	if((DTid.x % gCellSize) && (DTid.y % gCellSize)) //Inside the cell
	{
		uint2 cellNumber = DTid.xy / gCellSize.xx;
		uint  cellValue  = CellValue(cellNumber);

		float2 cellCoord     = DTid.xy - cellNumber * (float2)gCellSize.xx - (float2)gCellSize.xx / 2;
		float  diamondRadius = (gCellSize - 1) / 2;
		float  domainFactor  = 1.0f / (gDomainSize - 1.0f);

		bool insideCentralDiamond = (abs(cellCoord.x) + abs(cellCoord.y) <= (gCellSize - 1) / 2);

		bool insideHorizontalBeamLeft  = (abs(cellCoord.y) <= 0.707f * (gCellSize - 1) / 2 && cellCoord.x <= 0);
		bool insideHorizontalBeamRight = (abs(cellCoord.y) <= 0.707f * (gCellSize - 1) / 2 && cellCoord.x >= 0);
		bool insideVerticalBeamTop     = (abs(cellCoord.x) <= 0.707f * (gCellSize - 1) / 2 && cellCoord.y <= 0);
		bool insideVerticalBeamBottom  = (abs(cellCoord.x) <= 0.707f * (gCellSize - 1) / 2 && cellCoord.y >= 0);

		bool4 insideSide = bool4(cellCoord.x <= 0, cellCoord.y <= 0, cellCoord.x >= 0, cellCoord.y >= 0);

		bool4 insideBeam           = bool4(insideHorizontalBeamLeft, insideVerticalBeamTop, insideHorizontalBeamRight, insideVerticalBeamBottom);
		bool  insideCentralOctagon = insideCentralDiamond && (insideHorizontalBeamLeft || insideHorizontalBeamRight) && (insideVerticalBeamTop || insideVerticalBeamBottom);

		bool4 insideBeamCorner = insideBeam.xzzx &&  insideBeam.yyww && !insideCentralDiamond.xxxx;
		bool4 insideBeamEnd    = insideBeam.xyzw && !insideBeam.yxwz && !insideBeam.wzyx && insideCentralDiamond.xxxx;

		bool4 insideEdgeTopRightPart   = insideBeam.yyzz && !insideBeam.xzyw && !insideCentralDiamond.xxxx && insideSide.xzyw;
		bool4 insideEdgeBottomLeftPart = insideBeam.wwxx && !insideBeam.zxwy && !insideCentralDiamond.xxxx && insideSide.zxwy;
		
		bool4 insideCorner = !insideBeam.xyzw && !insideBeam.yzwx && insideSide.xyzw && insideSide.yzwx;

		uint leftPartValue        = (cellNumber.x > 0             )                                   * CellValue(cellNumber + int2(-1,  0));
		uint rightPartValue       = (cellNumber.x < gBoardSize - 1)                                   * CellValue(cellNumber + int2( 1,  0));
		uint topPartValue         = (cellNumber.y > 0             )                                   * CellValue(cellNumber + int2( 0, -1));
		uint bottomPartValue      = (cellNumber.y < gBoardSize - 1)                                   * CellValue(cellNumber + int2( 0,  1));
		uint leftTopPartValue     = (cellNumber.x > 0             ) * (cellNumber.y > 0             ) * CellValue(cellNumber + int2(-1, -1));
		uint rightTopPartValue    = (cellNumber.x < gBoardSize - 1) * (cellNumber.y > 0             ) * CellValue(cellNumber + int2( 1, -1));
		uint leftBottomPartValue  = (cellNumber.y < gBoardSize - 1) * (cellNumber.x > 0             ) * CellValue(cellNumber + int2(-1,  1));
		uint rightBottomPartValue = (cellNumber.x < gBoardSize - 1) * (cellNumber.y < gBoardSize - 1) * CellValue(cellNumber + int2( 1,  1));
		
		bool loneValue = cellValue != leftPartValue && cellValue != rightPartValue && cellValue != topPartValue && cellValue != bottomPartValue && cellValue != leftTopPartValue && cellValue != rightTopPartValue && cellValue != leftBottomPartValue && cellValue != rightBottomPartValue;

		uint4 edgeValue   = uint4(leftPartValue,    topPartValue,      rightPartValue,       bottomPartValue);
		uint4 cornerValue = uint4(leftTopPartValue, rightTopPartValue, rightBottomPartValue, leftBottomPartValue);

		uint4 emptyCornerCandidate = (edgeValue.xyzw == edgeValue.yzwx && edgeValue.xyzw != cornerValue.xyzw && edgeValue.xyzw != cellValue.xxxx) * edgeValue;
		
		uint4 beamCornerCandidate = ((cellValue.xxxx == edgeValue.xyzw) || (cellValue.xxxx == edgeValue.yzwx)                                       || (cellValue.xxxx == cornerValue.xyzw)                                     || (cellValue.xxxx == edgeValue.zwxy   && cellValue.xxxx == edgeValue.wxyz)                                                                           || (cellValue.xxxx == cornerValue.zwxy && cellValue.xxxx != cornerValue.wxyz && cellValue.xxxx != edgeValue.wxyz && cellValue.xxxx != edgeValue.zwxy && cellValue.xxxx != cornerValue.yzwx)                  ) * cellValue.xxxx;
		uint4 beamEndCandidate    = ((cellValue.xxxx == edgeValue.xyzw) || (cellValue.xxxx == cornerValue.xyzw && cellValue.xxxx != edgeValue.yzwx) || (cellValue.xxxx == cornerValue.wxyz && cellValue.xxxx != edgeValue.wxyz) || (cellValue.xxxx == cornerValue.zwxy && cellValue.xxxx == cornerValue.yzwx && cellValue.xxxx != edgeValue.wxyz && cellValue.xxxx != edgeValue.yzwx) || (cellValue.xxxx == edgeValue.zwxy   && cellValue.xxxx != edgeValue.wxyz   && cellValue.xxxx != edgeValue.yzwx)                                                                           || loneValue.xxxx) * cellValue.xxxx;

		uint4 edgeTopRightPartCandidate   = ((cellValue.xxxx == edgeValue.yyzz) || (cellValue.xxxx == cornerValue.xyyz && cellValue.xxxx != edgeValue.xzyw)) * cellValue.xxxx;
		uint4 edgeBottomLeftPartCandidate = ((cellValue.xxxx == edgeValue.wwxx) || (cellValue.xxxx == cornerValue.zwwx && cellValue.xxxx != edgeValue.zxwy)) * cellValue.xxxx;

		uint4 cornerCandidate = (cellValue.xxxx == cornerValue.xyzw && cellValue.xxxx != edgeValue.xyzw && cellValue.xxxx != edgeValue.yzwx) * cellValue.xxxx;

		uint4 resBeamCorner         = max(beamCornerCandidate,         emptyCornerCandidate.xyzw);
		uint4 resEdgeTopRightPart   = max(edgeTopRightPartCandidate,   emptyCornerCandidate.xyyz);
		uint4 resEdgeBottomLeftPart = max(edgeBottomLeftPartCandidate, emptyCornerCandidate.zwwx);
		uint4 resCorner             = max(cornerCandidate,             emptyCornerCandidate.xyzw);

		float  cellPower               = cellValue             * domainFactor;
		float4 beamCornerPower         = resBeamCorner         * domainFactor;
		float4 beamEndPower            = beamEndCandidate      * domainFactor;
		float4 edgeTopRightPartPower   = resEdgeTopRightPart   * domainFactor;
		float4 edgeBottomLeftPartPower = resEdgeBottomLeftPart * domainFactor;
		float4 cornerPower             = resCorner             * domainFactor;

		float enablePower = cellPower * insideCentralOctagon + dot(beamCornerPower, insideBeamCorner) + dot(beamEndPower, insideBeamEnd) + dot(edgeTopRightPartPower, insideEdgeTopRightPart) + dot(edgeBottomLeftPartPower, insideEdgeBottomLeftPart) + dot(cornerPower, insideCorner);
		result            = lerp(gColorNone, gColorEnabled, enablePower);

		[flatten]
		if(gFlags & FLAG_SHOW_SOLUTION) //Show the solution
		{
			uint solutionValue = CellValueSolution(cellNumber);

			uint leftPartSolved        = (cellNumber.x > 0             )                                   * CellValueSolution(cellNumber + int2(-1,  0));
			uint rightPartSolved       = (cellNumber.x < gBoardSize - 1)                                   * CellValueSolution(cellNumber + int2( 1,  0));
			uint topPartSolved         = (cellNumber.y > 0             )                                   * CellValueSolution(cellNumber + int2( 0, -1));
			uint bottomPartSolved      = (cellNumber.y < gBoardSize - 1)                                   * CellValueSolution(cellNumber + int2( 0,  1));
			uint leftTopPartSolved     = (cellNumber.x > 0             ) * (cellNumber.y > 0             ) * CellValueSolution(cellNumber + int2(-1, -1));
			uint rightTopPartSolved    = (cellNumber.x < gBoardSize - 1) * (cellNumber.y > 0             ) * CellValueSolution(cellNumber + int2( 1, -1));
			uint leftBottomPartSolved  = (cellNumber.y < gBoardSize - 1) * (cellNumber.x > 0             ) * CellValueSolution(cellNumber + int2(-1,  1));
			uint rightBottomPartSolved = (cellNumber.x < gBoardSize - 1) * (cellNumber.y < gBoardSize - 1) * CellValueSolution(cellNumber + int2( 1,  1));

			bool loneSolved = solutionValue != leftPartSolved && solutionValue != rightPartSolved && solutionValue != topPartSolved && solutionValue != bottomPartSolved && solutionValue != leftTopPartSolved && solutionValue != rightTopPartSolved && solutionValue != leftBottomPartSolved && solutionValue != rightBottomPartSolved;

			uint4 edgeSolved   = uint4(leftPartSolved,    topPartSolved,      rightPartSolved,       bottomPartSolved);
			uint4 cornerSolved = uint4(leftTopPartSolved, rightTopPartSolved, rightBottomPartSolved, leftBottomPartSolved);

			uint4 emptyCornerSolutionCandidate = (edgeSolved.xyzw == edgeSolved.yzwx && edgeSolved.xyzw != cornerSolved.xyzw && edgeSolved.xyzw != solutionValue.xxxx) * edgeSolved;
		
			uint4 beamCornerSolutionCandidate = ((solutionValue.xxxx == edgeSolved.xyzw) || (solutionValue.xxxx == edgeSolved.yzwx)                                            || (solutionValue.xxxx == cornerSolved.xyzw)                                          || (solutionValue.xxxx == edgeSolved.zwxy   && solutionValue.xxxx == edgeSolved.wxyz)                                                                                     || (solutionValue.xxxx == cornerSolved.zwxy && solutionValue.xxxx != cornerSolved.wxyz && solutionValue.xxxx != edgeSolved.wxyz && solutionValue.xxxx != edgeSolved.zwxy && solutionValue.xxxx != cornerSolved.yzwx)                  ) * solutionValue.xxxx;
			uint4 beamEndSolutionCandidate    = ((solutionValue.xxxx == edgeSolved.xyzw) || (solutionValue.xxxx == cornerSolved.xyzw && solutionValue.xxxx != edgeSolved.yzwx) || (solutionValue.xxxx == cornerSolved.wxyz && solutionValue.xxxx != edgeSolved.wxyz) || (solutionValue.xxxx == cornerSolved.zwxy && solutionValue.xxxx == cornerSolved.yzwx && solutionValue.xxxx != edgeSolved.wxyz && solutionValue.xxxx != edgeSolved.yzwx) || (solutionValue.xxxx == edgeSolved.zwxy   && solutionValue.xxxx != edgeSolved.wxyz   && solutionValue.xxxx != edgeSolved.yzwx)                                                                                    || loneSolved.xxxx) * solutionValue.xxxx;

			uint4 edgeTopRightPartSolutionCandidate   = ((solutionValue.xxxx == edgeSolved.yyzz) || (solutionValue.xxxx == cornerSolved.xyyz && solutionValue.xxxx != edgeSolved.xzyw)) * solutionValue.xxxx;
			uint4 edgeBottomLeftPartSolutionCandidate = ((solutionValue.xxxx == edgeSolved.wwxx) || (solutionValue.xxxx == cornerSolved.zwwx && solutionValue.xxxx != edgeSolved.zxwy)) * solutionValue.xxxx;

			uint4 cornerSolutionCandidate = (solutionValue.xxxx == cornerSolved.xyzw && solutionValue.xxxx != edgeSolved.xyzw && solutionValue.xxxx != edgeSolved.yzwx) * solutionValue.xxxx;

			uint4 resBeamCornerSolved         = max(beamCornerSolutionCandidate,         emptyCornerSolutionCandidate.xyzw);
			uint4 resEdgeTopRightPartSolved   = max(edgeTopRightPartSolutionCandidate,   emptyCornerSolutionCandidate.xyyz);
			uint4 resEdgeBottomLeftPartSolved = max(edgeBottomLeftPartSolutionCandidate, emptyCornerSolutionCandidate.zwwx);
			uint4 resCornerSolved             = max(cornerSolutionCandidate,             emptyCornerSolutionCandidate.xyzw);

			float  cellSolutionPower               = solutionValue               * domainFactor;
			float4 beamCornerSolutionPower         = resBeamCornerSolved         * domainFactor;
			float4 beamEndSolutionPower            = beamEndSolutionCandidate    * domainFactor;
			float4 edgeTopRightPartSolutionPower   = resEdgeTopRightPartSolved   * domainFactor;
			float4 edgeBottomLeftPartSolutionPower = resEdgeBottomLeftPartSolved * domainFactor;
			float4 cornerSolutionPower             = resCornerSolved             * domainFactor;

			float solvedPower = cellSolutionPower * insideCentralOctagon + dot(beamCornerSolutionPower, insideBeamCorner) + dot(beamEndSolutionPower, insideBeamEnd) + dot(edgeTopRightPartSolutionPower, insideEdgeTopRightPart) + dot(edgeBottomLeftPartSolutionPower, insideEdgeBottomLeftPart) + dot(cornerSolutionPower, insideCorner);
			result            = lerp(result, gColorSolved, solvedPower);
		}
		else if(gFlags & FLAG_SHOW_STABILITY)
		{
			uint   stableValue = CellValueStability(cellNumber);
			float4 colorStable = float4(1.0f, 1.0f, 1.0f, 1.0f) - gColorEnabled;

			uint leftPartStable        = (cellNumber.x > 0             )                                   * CellValueStability(cellNumber + int2(-1,  0));
			uint rightPartStable       = (cellNumber.x < gBoardSize - 1)                                   * CellValueStability(cellNumber + int2( 1,  0));
			uint topPartStable         = (cellNumber.y > 0             )                                   * CellValueStability(cellNumber + int2( 0, -1));
			uint bottomPartStable      = (cellNumber.y < gBoardSize - 1)                                   * CellValueStability(cellNumber + int2( 0,  1));
			uint leftTopPartStable     = (cellNumber.x > 0             ) * (cellNumber.y > 0             ) * CellValueStability(cellNumber + int2(-1, -1));
			uint rightTopPartStable    = (cellNumber.x < gBoardSize - 1) * (cellNumber.y > 0             ) * CellValueStability(cellNumber + int2( 1, -1));
			uint leftBottomPartStable  = (cellNumber.y < gBoardSize - 1) * (cellNumber.x > 0             ) * CellValueStability(cellNumber + int2(-1,  1));
			uint rightBottomPartStable = (cellNumber.x < gBoardSize - 1) * (cellNumber.y < gBoardSize - 1) * CellValueStability(cellNumber + int2( 1,  1));

			bool loneStable = stableValue != leftPartStable && stableValue != rightPartStable && stableValue != topPartStable && stableValue != bottomPartStable && stableValue != leftTopPartStable && stableValue != rightTopPartStable && stableValue != leftBottomPartStable && stableValue != rightBottomPartStable;

			uint4 edgeStable   = uint4(leftPartStable,    topPartStable,      rightPartStable,       bottomPartStable);
			uint4 cornerStable = uint4(leftTopPartStable, rightTopPartStable, rightBottomPartStable, leftBottomPartStable);

			uint4 emptyCornerStabilityCandidate = (edgeStable.xyzw == edgeStable.yzwx && edgeStable.xyzw != cornerStable.xyzw && edgeStable.xyzw != stableValue.xxxx) * edgeStable;
		
			uint4 beamCornerStabilityCandidate = ((stableValue.xxxx == edgeStable.xyzw) || (stableValue.xxxx == edgeStable.yzwx)                                          || (stableValue.xxxx == cornerStable.xyzw)                                        || (stableValue.xxxx == edgeStable.zwxy   && stableValue.xxxx == edgeStable.wxyz)                                                                                 || (stableValue.xxxx == cornerStable.zwxy && stableValue.xxxx != cornerStable.wxyz && stableValue.xxxx != edgeStable.wxyz && stableValue.xxxx != edgeStable.zwxy && stableValue.xxxx != cornerStable.yzwx)                  ) * stableValue.xxxx;
			uint4 beamEndStabilityCandidate    = ((stableValue.xxxx == edgeStable.xyzw) || (stableValue.xxxx == cornerStable.xyzw && stableValue.xxxx != edgeStable.yzwx) || (stableValue.xxxx == cornerStable.wxyz && stableValue.xxxx != edgeStable.wxyz) || (stableValue.xxxx == cornerStable.zwxy && stableValue.xxxx == cornerStable.yzwx && stableValue.xxxx != edgeStable.wxyz && stableValue.xxxx != edgeStable.yzwx) || (stableValue.xxxx == edgeStable.zwxy   && stableValue.xxxx != edgeStable.wxyz   && stableValue.xxxx != edgeStable.yzwx)                                                                                || loneStable.xxxx) * stableValue.xxxx;

			uint4 edgeTopRightPartStabilityCandidate   = ((stableValue.xxxx == edgeStable.yyzz) || (stableValue.xxxx == cornerStable.xyyz && stableValue.xxxx != edgeStable.xzyw)) * stableValue.xxxx;
			uint4 edgeBottomLeftPartStabilityCandidate = ((stableValue.xxxx == edgeStable.wwxx) || (stableValue.xxxx == cornerStable.zwwx && stableValue.xxxx != edgeStable.zxwy)) * stableValue.xxxx;

			uint4 cornerStabilityCandidate = (stableValue.xxxx == cornerStable.xyzw && stableValue.xxxx != edgeStable.xyzw && stableValue.xxxx != edgeStable.yzwx) * stableValue.xxxx;

			uint4 resBeamCornerStable         = max(beamCornerStabilityCandidate,         emptyCornerStabilityCandidate.xyzw);
			uint4 resEdgeTopRightPartStable   = max(edgeTopRightPartStabilityCandidate,   emptyCornerStabilityCandidate.xyyz);
			uint4 resEdgeBottomLeftPartStable = max(edgeBottomLeftPartStabilityCandidate, emptyCornerStabilityCandidate.zwwx);
			uint4 resCornerStable             = max(cornerStabilityCandidate,             emptyCornerStabilityCandidate.xyzw);

			float  cellStabilityPower               = stableValue                 * domainFactor;
			float4 beamCornerStabilityPower         = resBeamCornerStable         * domainFactor;
			float4 beamEndStabilityPower            = beamEndStabilityCandidate   * domainFactor;
			float4 edgeTopRightPartStabilityPower   = resEdgeTopRightPartStable   * domainFactor;
			float4 edgeBottomLeftPartStabilityPower = resEdgeBottomLeftPartStable * domainFactor;
			float4 cornerStabilityPower             = resCornerStable             * domainFactor;

			float stablePower = cellStabilityPower * insideCentralOctagon + dot(beamCornerStabilityPower, insideBeamCorner) + dot(beamEndStabilityPower, insideBeamEnd) + dot(edgeTopRightPartStabilityPower, insideEdgeTopRightPart) + dot(edgeBottomLeftPartStabilityPower, insideEdgeBottomLeftPart) + dot(cornerStabilityPower, insideCorner);
			result            = lerp(result, colorStable, stablePower);
		}
	}
	else //Edge of the cell
	{
		result = gColorBetween;
	}

	Result[DTid.xy] = result;
}