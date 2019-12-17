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

		int iBoardSize = (int)gBoardSize;
		int iCellSize  = (int)gCellSize;

		float2 cellCoord       = DTid.xy - cellNumber * (float2)gCellSize.xx - (float2)gCellSize.xx / 2;
		float  circleRadius    = (gCellSize - 1) / 2;
		float  circleRadiusBig = (gCellSize - 1);
		float  domainFactor    = 1.0f / (gDomainSize - 1.0f);

		float2 cellCoordLeft        = cellCoord + float2( iCellSize,          0);
		float2 cellCoordRight       = cellCoord + float2(-iCellSize,          0);
		float2 cellCoordTop         = cellCoord + float2(         0,  iCellSize);
		float2 cellCoordBottom      = cellCoord + float2(         0, -iCellSize);

		bool insideCircle     = (dot(cellCoord, cellCoord) < circleRadius * circleRadius);
		bool insideCircleBigL = (dot(       cellCoordLeft,        cellCoordLeft) < circleRadiusBig * circleRadiusBig);
		bool insideCircleBigR = (dot(      cellCoordRight,       cellCoordRight) < circleRadiusBig * circleRadiusBig);
		bool insideCircleBigT = (dot(        cellCoordTop,         cellCoordTop) < circleRadiusBig * circleRadiusBig);
		bool insideCircleBigB = (dot(     cellCoordBottom,      cellCoordBottom) < circleRadiusBig * circleRadiusBig);

		bool insideTopLeft     = !insideCircle && cellCoord.x <= 0 && cellCoord.y <= 0;
		bool insideTopRight    = !insideCircle && cellCoord.x >= 0 && cellCoord.y <= 0;
		bool insideBottomRight = !insideCircle && cellCoord.x >= 0 && cellCoord.y >= 0;
		bool insideBottomLeft  = !insideCircle && cellCoord.x <= 0 && cellCoord.y >= 0;

		bool4 insideCircleBig = bool4(insideCircleBigL, insideCircleBigT, insideCircleBigR,  insideCircleBigB);
		bool4 insideCorner    = bool4(insideTopLeft,    insideTopRight,   insideBottomRight, insideBottomLeft);

		bool insideLinkH = !insideCircleBigT && !insideCircleBigB;
		bool insideLinkV = !insideCircleBigL && !insideCircleBigR;

		bool2 insideLink      = bool2(insideLinkH, insideLinkV);
		bool1 insideBothLinks = insideLinkV && insideLinkH;

		bool4 insideSlimEdgeTopRightPart   = insideLink.xyyx && insideCorner.xxyy;
		bool4 insideSlimEdgeBottomLeftPart = insideLink.xyyx && insideCorner.zzww;

		bool2 insideCenterLink = insideLink   && insideCircle.xx  && !insideBothLinks.xx;
		bool1 insideFreeCircle = insideCircle && !insideLinkV     && !insideLinkH;
		bool4 insideFreeCorner = insideCorner && !insideLink.xxxx && !insideLink.yyyy;

		int2 leftCell         = cellNumber + int2(-1,  0);
		int2 rightCell        = cellNumber + int2( 1,  0);
		int2 topCell          = cellNumber + int2( 0, -1);
		int2 bottomCell       = cellNumber + int2( 0,  1);
		int2 leftTopCell      = cellNumber + int2(-1, -1);
		int2 rightTopCell     = cellNumber + int2( 1, -1);
		int2 leftBottomCell   = cellNumber + int2(-1,  1);
		int2 rightBottomCell  = cellNumber + int2( 1,  1);
		int2 left2Cell        = cellNumber + int2(-2,  0);
		int2 right2Cell       = cellNumber + int2( 2,  0);
		int2 top2Cell         = cellNumber + int2( 0, -2);
		int2 bottom2Cell      = cellNumber + int2( 0,  2);

		bool nonLeftEdge        = cellNumber.x > 0;
		bool nonRightEdge       = cellNumber.x < gBoardSize - 1;
		bool nonTopEdge         =                                  cellNumber.y > 0;
		bool nonBottomEdge      =                                  cellNumber.y < gBoardSize - 1;
		bool nonLeftTopEdge     = cellNumber.x > 0              && cellNumber.y > 0;
		bool nonRightTopEdge    = cellNumber.x < gBoardSize - 1 && cellNumber.y > 0;
		bool nonLeftBottomEdge  = cellNumber.x > 0              && cellNumber.y < gBoardSize - 1;
		bool nonRightBottomEdge = cellNumber.x < gBoardSize - 1 && cellNumber.y < gBoardSize - 1;
		bool nonLeft2Edge       = cellNumber.x > 1;
		bool nonRight2Edge      = cellNumber.x < gBoardSize - 2;
		bool nonTop2Edge        =                                  cellNumber.y > 1;
		bool nonBottom2Edge     =                                  cellNumber.y < gBoardSize - 2;

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
			nonLeft2Edge       = true;
			nonRight2Edge      = true;
			nonTop2Edge        = true;
			nonBottom2Edge     = true;

			const uint maxCheckDistance = 2; //Different for different render modes

			uint2 leftCellU        = (uint2)(leftCell        + gBoardSize.xx * maxCheckDistance);
			uint2 rightCellU       = (uint2)(rightCell       + gBoardSize.xx * maxCheckDistance);
			uint2 topCellU         = (uint2)(topCell         + gBoardSize.xx * maxCheckDistance);
			uint2 bottomCellU      = (uint2)(bottomCell      + gBoardSize.xx * maxCheckDistance);
			uint2 leftTopCellU     = (uint2)(leftTopCell     + gBoardSize.xx * maxCheckDistance);
			uint2 rightTopCellU    = (uint2)(rightTopCell    + gBoardSize.xx * maxCheckDistance);
			uint2 leftBottomCellU  = (uint2)(leftBottomCell  + gBoardSize.xx * maxCheckDistance);
			uint2 rightBottomCellU = (uint2)(rightBottomCell + gBoardSize.xx * maxCheckDistance);
			uint2 left2CellU       = (uint2)(left2Cell       + gBoardSize.xx * maxCheckDistance);
			uint2 right2CellU      = (uint2)(right2Cell      + gBoardSize.xx * maxCheckDistance);
			uint2 top2CellU        = (uint2)(top2Cell        + gBoardSize.xx * maxCheckDistance);
			uint2 bottom2CellU     = (uint2)(bottom2Cell     + gBoardSize.xx * maxCheckDistance);

			leftCell        = (int2)(leftCellU        % gBoardSize.xx);
			rightCell       = (int2)(rightCellU       % gBoardSize.xx);
			topCell         = (int2)(topCellU         % gBoardSize.xx);
			bottomCell      = (int2)(bottomCellU      % gBoardSize.xx);
			leftTopCell     = (int2)(leftTopCellU     % gBoardSize.xx);
			rightTopCell    = (int2)(rightTopCellU    % gBoardSize.xx);
			leftBottomCell  = (int2)(leftBottomCellU  % gBoardSize.xx);
			rightBottomCell = (int2)(rightBottomCellU % gBoardSize.xx);
			left2Cell       = (int2)(left2CellU       % gBoardSize.xx);
			right2Cell      = (int2)(right2CellU      % gBoardSize.xx);
			top2Cell        = (int2)(top2CellU        % gBoardSize.xx);
			bottom2Cell     = (int2)(bottom2CellU     % gBoardSize.xx);
		}

		uint leftPartValue         = nonLeftEdge        * CellValue(leftCell);
		uint rightPartValue        = nonRightEdge       * CellValue(rightCell);
		uint topPartValue          = nonTopEdge         * CellValue(topCell);
		uint bottomPartValue       = nonBottomEdge      * CellValue(bottomCell);
		uint leftTopPartValue      = nonLeftTopEdge     * CellValue(leftTopCell);
		uint rightTopPartValue     = nonRightTopEdge    * CellValue(rightTopCell);
		uint leftBottomPartValue   = nonLeftBottomEdge  * CellValue(leftBottomCell);
		uint rightBottomPartValue  = nonRightBottomEdge * CellValue(rightBottomCell);
		uint left2PartValue        = nonLeft2Edge       * CellValue(left2Cell);
		uint right2PartValue       = nonRight2Edge      * CellValue(right2Cell);
		uint top2PartValue         = nonTop2Edge        * CellValue(top2Cell);
		uint bottom2PartValue      = nonBottom2Edge     * CellValue(bottom2Cell);

		uint4 edgeValue   = uint4(leftPartValue,    topPartValue,      rightPartValue,       bottomPartValue);
		uint4 cornerValue = uint4(leftTopPartValue, rightTopPartValue, rightBottomPartValue, leftBottomPartValue);
		uint4 edge2Value  = uint4(left2PartValue,   top2PartValue,     right2PartValue,      bottom2PartValue);

		uint CenterCandidate = cellValue;

		uint4 emptyCornerCandidate = (edgeValue.xyzw == edgeValue.yzwx                                                                          ) * edgeValue;
		uint4 cornerCandidate      = (cellValue.xxxx == cornerValue.xyzw || cellValue.xxxx == edgeValue.xyzw || cellValue.xxxx == edgeValue.yzwx) * cellValue.xxxx;

		uint2 linkCandidate     = (edgeValue.xy   == edgeValue.zw   ) * edgeValue.xy;
		uint4 slimEdgeCandidate = (cellValue.xxxx == edge2Value.xyzw) * cellValue.xxxx;

		uint4 resCorner                  = max(cornerCandidate, emptyCornerCandidate);
		uint4 resSlimCornerTopRightPart  = max(resCorner.xxyy,  slimEdgeCandidate.xyyz);
		uint4 resSlimCornerBotomLeftPart = max(resCorner.zzww,  slimEdgeCandidate.zwwx);

		uint2 resLink     = max(linkCandidate, CenterCandidate.xx);
		uint1 resMidLinks = max(resLink.x, resLink.y);

		float  cellPower           = cellValue                  * domainFactor;
		float4 cornerPower         = resCorner                  * domainFactor.xxxx;
		float4 slimTopRightPower   = resSlimCornerTopRightPart  * domainFactor.xxxx;
		float4 slimBottomLeftPower = resSlimCornerBotomLeftPart * domainFactor.xxxx;
		float2 linkPower           = resLink                    * domainFactor.xx;
		float  midPower            = resMidLinks                * domainFactor;

		float enablePower = cellPower * insideFreeCircle + dot(insideFreeCorner, cornerPower) + dot(slimTopRightPower, insideSlimEdgeTopRightPart) + dot(slimBottomLeftPower, insideSlimEdgeBottomLeftPart) + dot(linkPower, insideCenterLink) + midPower * insideBothLinks;
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
			uint left2PartSolved       = nonLeft2Edge       * CellValueSolution(left2Cell);
			uint right2PartSolved      = nonRight2Edge      * CellValueSolution(right2Cell);
			uint top2PartSolved        = nonTop2Edge        * CellValueSolution(top2Cell);
			uint bottom2PartSolved     = nonBottom2Edge     * CellValueSolution(bottom2Cell);

			uint4 edgeSolved   = uint4(leftPartSolved,    topPartSolved,      rightPartSolved,       bottomPartSolved);
			uint4 cornerSolved = uint4(leftTopPartSolved, rightTopPartSolved, rightBottomPartSolved, leftBottomPartSolved);
			uint4 edge2Solved  = uint4(left2PartSolved,   top2PartSolved,     right2PartSolved,      bottom2PartSolved);

			uint CenterSolutionCandidate = solutionValue;

			uint4 emptyCornerSolutionCandidate = (edgeSolved.xyzw    == edgeSolved.yzwx                                                                                    ) * edgeSolved;
			uint4 cornerSolutionCandidate      = (solutionValue.xxxx == cornerSolved.xyzw || solutionValue.xxxx == edgeSolved.xyzw || solutionValue.xxxx == edgeSolved.yzwx) * solutionValue.xxxx;

			uint2 linkSolutionCandidate     = (edgeSolved.xy      == edgeSolved.zw   ) * edgeSolved.xy;
			uint4 slimEdgeSolutionCandidate = (solutionValue.xxxx == edge2Solved.xyzw) * solutionValue.xxxx;

			uint4 resCornerSolution                  = max(cornerSolutionCandidate, emptyCornerSolutionCandidate);
			uint4 resSlimCornerTopRightPartSolution  = max(resCornerSolution.xxyy,  slimEdgeSolutionCandidate.xyyz);
			uint4 resSlimCornerBotomLeftPartSolution = max(resCornerSolution.zzww,  slimEdgeSolutionCandidate.zwwx);

			uint2 resLinkSolution     = max(linkSolutionCandidate, CenterSolutionCandidate.xx);
			uint1 resMidLinksSolution = max(resLinkSolution.x,     resLinkSolution.y);

			float  cellSolutionPower           = solutionValue                      * domainFactor;
			float4 cornerSolutionPower         = resCornerSolution                  * domainFactor.xxxx;
			float4 slimTopRightSolutionPower   = resSlimCornerTopRightPartSolution  * domainFactor.xxxx;
			float4 slimBottomLeftSolutionPower = resSlimCornerBotomLeftPartSolution * domainFactor.xxxx;
			float2 linkSolutionPower           = resLinkSolution                    * domainFactor.xx;
			float  midSolutionPower            = resMidLinksSolution                * domainFactor;

			float solutionPower = cellSolutionPower * insideFreeCircle + dot(insideFreeCorner, cornerSolutionPower) + dot(slimTopRightSolutionPower, insideSlimEdgeTopRightPart) + dot(slimBottomLeftSolutionPower, insideSlimEdgeBottomLeftPart) + dot(linkSolutionPower, insideCenterLink) + midSolutionPower * insideBothLinks;
			result              = lerp(result, gColorSolved, solutionPower);
		}
		else if(gFlags & FLAG_SHOW_STABILITY)
		{
			uint   stabilityValue = CellValueStability(cellNumber);
			float4 colorStable    = float4(1.0f, 1.0f, 1.0f, 1.0f) - gColorEnabled;

			uint leftPartStable        = nonLeftEdge        * CellValueStability(leftCell);
			uint rightPartStable       = nonRightEdge       * CellValueStability(rightCell);
			uint topPartStable         = nonTopEdge         * CellValueStability(topCell);
			uint bottomPartStable      = nonBottomEdge      * CellValueStability(bottomCell);
			uint leftTopPartStable     = nonLeftTopEdge     * CellValueStability(leftTopCell);
			uint rightTopPartStable    = nonRightTopEdge    * CellValueStability(rightTopCell);
			uint leftBottomPartStable  = nonLeftBottomEdge  * CellValueStability(leftBottomCell);
			uint rightBottomPartStable = nonRightBottomEdge * CellValueStability(rightBottomCell);
			uint left2PartStable       = nonLeft2Edge       * CellValueStability(left2Cell);
			uint right2PartStable      = nonRight2Edge      * CellValueStability(right2Cell);
			uint top2PartStable        = nonTop2Edge        * CellValueStability(top2Cell);
			uint bottom2PartStable     = nonBottom2Edge     * CellValueStability(bottom2Cell);

			uint4 edgeStable   = uint4(leftPartStable,    topPartStable,      rightPartStable,       bottomPartStable);
			uint4 cornerStable = uint4(leftTopPartStable, rightTopPartStable, rightBottomPartStable, leftBottomPartStable);
			uint4 edge2Stable  = uint4(left2PartStable,   top2PartStable,     right2PartStable,      bottom2PartStable);

			uint CenterStabilityCandidate = stabilityValue;

			uint4 emptyCornerStabilityCandidate = (edgeStable.xyzw     == edgeStable.yzwx                                                                                      ) * edgeStable;
			uint4 cornerStabilityCandidate      = (stabilityValue.xxxx == cornerStable.xyzw || stabilityValue.xxxx == edgeStable.xyzw || stabilityValue.xxxx == edgeStable.yzwx) * stabilityValue.xxxx;

			uint2 linkStabilityCandidate     = (edgeStable.xy       == edgeStable.zw   ) * edgeStable.xy;
			uint4 slimEdgeStabilityCandidate = (stabilityValue.xxxx == edge2Stable.xyzw) * stabilityValue.xxxx;

			uint4 resCornerStability                  = max(cornerStabilityCandidate, emptyCornerStabilityCandidate);
			uint4 resSlimCornerTopRightPartStability  = max(resCornerStability.xxyy,  slimEdgeStabilityCandidate.xyyz);
			uint4 resSlimCornerBotomLeftPartStability = max(resCornerStability.zzww,  slimEdgeStabilityCandidate.zwwx);

			uint2 resLinkStability     = max(linkStabilityCandidate, CenterStabilityCandidate.xx);
			uint1 resMidLinksStability = max(resLinkStability.x,     resLinkStability.y);

			float  cellStabilityPower           = stabilityValue                      * domainFactor;
			float4 cornerStabilityPower         = resCornerStability                  * domainFactor.xxxx;
			float4 slimTopRightStabilityPower   = resSlimCornerTopRightPartStability  * domainFactor.xxxx;
			float4 slimBottomLeftStabilityPower = resSlimCornerBotomLeftPartStability * domainFactor.xxxx;
			float2 linkStabilityPower           = resLinkStability                    * domainFactor.xx;
			float  midStabilityPower            = resMidLinksStability                * domainFactor;

			float stablePower = cellStabilityPower * insideFreeCircle + dot(insideFreeCorner, cornerStabilityPower) + dot(slimTopRightStabilityPower, insideSlimEdgeTopRightPart) + dot(slimBottomLeftStabilityPower, insideSlimEdgeBottomLeftPart) + dot(linkStabilityPower, insideCenterLink) + midStabilityPower * insideBothLinks;
			result            = lerp(result, colorStable, stablePower);
		}
	}
	else //Edge of the cell
	{
		result = gColorBetween;
	}

	Result[DTid.xy] = result;
}