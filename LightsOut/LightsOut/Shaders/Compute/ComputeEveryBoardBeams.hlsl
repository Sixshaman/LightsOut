#define FLAG_SHOW_SOLUTION  0x01
#define FLAG_SHOW_STABILITY 0x02
#define FLAG_TOROID_RENDER  0x04

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

		int2 leftCell        = cellNumber + int2(-1,  0);
		int2 rightCell       = cellNumber + int2( 1,  0);
		int2 topCell         = cellNumber + int2( 0, -1);
		int2 bottomCell      = cellNumber + int2( 0,  1);
		int2 leftTopCell     = cellNumber + int2(-1, -1);
		int2 rightTopCell    = cellNumber + int2( 1, -1);
		int2 leftBottomCell  = cellNumber + int2(-1,  1);
		int2 rightBottomCell = cellNumber + int2( 1,  1);

		bool nonLeftEdge        = cellNumber.x > 0;
		bool nonRightEdge       = cellNumber.x < gBoardSize - 1;
		bool nonTopEdge         =                                  cellNumber.y > 0;
		bool nonBottomEdge      =                                  cellNumber.y < gBoardSize - 1;
		bool nonLeftTopEdge     = cellNumber.x > 0              && cellNumber.y > 0;
		bool nonRightTopEdge    = cellNumber.x < gBoardSize - 1 && cellNumber.y > 0;
		bool nonLeftBottomEdge  = cellNumber.x > 0              && cellNumber.y < gBoardSize - 1;
		bool nonRightBottomEdge = cellNumber.x < gBoardSize - 1 && cellNumber.y < gBoardSize - 1;

		if(gFlags & FLAG_TOROID_RENDER)
		{
			nonLeftEdge        = true;
			nonRightEdge       = true;
			nonTopEdge         = true;
			nonBottomEdge      = true;
			nonLeftTopEdge     = true;
			nonRightTopEdge    = true;
			nonLeftBottomEdge  = true;
			nonRightBottomEdge = true;

			const uint maxCheckDistance = 1; //Different for different render modes

			uint2 leftCellU        = (uint2)(leftCell        + gBoardSize.xx * maxCheckDistance);
			uint2 rightCellU       = (uint2)(rightCell       + gBoardSize.xx * maxCheckDistance);
			uint2 topCellU         = (uint2)(topCell         + gBoardSize.xx * maxCheckDistance);
			uint2 bottomCellU      = (uint2)(bottomCell      + gBoardSize.xx * maxCheckDistance);
			uint2 leftTopCellU     = (uint2)(leftTopCell     + gBoardSize.xx * maxCheckDistance);
			uint2 rightTopCellU    = (uint2)(rightTopCell    + gBoardSize.xx * maxCheckDistance);
			uint2 leftBottomCellU  = (uint2)(leftBottomCell  + gBoardSize.xx * maxCheckDistance);
			uint2 rightBottomCellU = (uint2)(rightBottomCell + gBoardSize.xx * maxCheckDistance);

			leftCell        = (int2)(leftCellU        % gBoardSize.xx);
			rightCell       = (int2)(rightCellU       % gBoardSize.xx);
			topCell         = (int2)(topCellU         % gBoardSize.xx);
			bottomCell      = (int2)(bottomCellU      % gBoardSize.xx);
			leftTopCell     = (int2)(leftTopCellU     % gBoardSize.xx);
			rightTopCell    = (int2)(rightTopCellU    % gBoardSize.xx);
			leftBottomCell  = (int2)(leftBottomCellU  % gBoardSize.xx);
			rightBottomCell = (int2)(rightBottomCellU % gBoardSize.xx);
		}

		uint leftPartValue        = nonLeftEdge        * CellValue(leftCell);
		uint rightPartValue       = nonRightEdge       * CellValue(rightCell);
		uint topPartValue         = nonTopEdge         * CellValue(topCell);
		uint bottomPartValue      = nonBottomEdge      * CellValue(bottomCell);
		uint leftTopPartValue     = nonLeftTopEdge     * CellValue(leftTopCell);
		uint rightTopPartValue    = nonRightTopEdge    * CellValue(rightTopCell);
		uint leftBottomPartValue  = nonLeftBottomEdge  * CellValue(leftBottomCell);
		uint rightBottomPartValue = nonRightBottomEdge * CellValue(rightBottomCell);
		
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

			uint leftPartSolved        = nonLeftEdge        * CellValueSolution(leftCell);
			uint rightPartSolved       = nonRightEdge       * CellValueSolution(rightCell);
			uint topPartSolved         = nonTopEdge         * CellValueSolution(topCell);
			uint bottomPartSolved      = nonBottomEdge      * CellValueSolution(bottomCell);
			uint leftTopPartSolved     = nonLeftTopEdge     * CellValueSolution(leftTopCell);
			uint rightTopPartSolved    = nonRightTopEdge    * CellValueSolution(rightTopCell);
			uint leftBottomPartSolved  = nonLeftBottomEdge  * CellValueSolution(leftBottomCell);
			uint rightBottomPartSolved = nonRightBottomEdge * CellValueSolution(rightBottomCell);

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

			uint leftPartStable        = nonLeftEdge        * CellValueStability(leftCell);
			uint rightPartStable       = nonRightEdge       * CellValueStability(rightCell);
			uint topPartStable         = nonTopEdge         * CellValueStability(topCell);
			uint bottomPartStable      = nonBottomEdge      * CellValueStability(bottomCell);
			uint leftTopPartStable     = nonLeftTopEdge     * CellValueStability(leftTopCell);
			uint rightTopPartStable    = nonRightTopEdge    * CellValueStability(rightTopCell);
			uint leftBottomPartStable  = nonLeftBottomEdge  * CellValueStability(leftBottomCell);
			uint rightBottomPartStable = nonRightBottomEdge * CellValueStability(rightBottomCell);

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