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

		int iBoardSize = (int)gBoardSize;
		int iCellSize  = (int)gCellSize;

		float2 cellCoord       = DTid.xy - cellNumber * (float2)gCellSize.xx - (float2)gCellSize.xx / 2;
		float  circleRadius    = (gCellSize - 1) / 2;
		float  circleRadiusBig = (gCellSize - 1);

		float2 cellCoordLeft        = cellCoord + float2( iCellSize,          0);
		float2 cellCoordRight       = cellCoord + float2(-iCellSize,          0);
		float2 cellCoordTop         = cellCoord + float2(         0,  iCellSize);
		float2 cellCoordBottom      = cellCoord + float2(         0, -iCellSize);

		bool insideCircle          = (dot(cellCoord, cellCoord) < circleRadius * circleRadius);
		bool insideCircleBigL      = (dot(       cellCoordLeft,        cellCoordLeft) < circleRadiusBig * circleRadiusBig);
		bool insideCircleBigR      = (dot(      cellCoordRight,       cellCoordRight) < circleRadiusBig * circleRadiusBig);
		bool insideCircleBigT      = (dot(        cellCoordTop,         cellCoordTop) < circleRadiusBig * circleRadiusBig);
		bool insideCircleBigB      = (dot(     cellCoordBottom,      cellCoordBottom) < circleRadiusBig * circleRadiusBig);
		bool insideTopLeft         = !insideCircle && cellCoord.x <= 0 && cellCoord.y <= 0;
		bool insideTopRight        = !insideCircle && cellCoord.x >= 0 && cellCoord.y <= 0;
		bool insideBottomRight     = !insideCircle && cellCoord.x >= 0 && cellCoord.y >= 0;
		bool insideBottomLeft      = !insideCircle && cellCoord.x <= 0 && cellCoord.y >= 0;
		bool insideLinkV           = !insideCircleBigL && !insideCircleBigR;
		bool insideLinkH           = !insideCircleBigT && !insideCircleBigB;
		bool insideBothLinks       = insideLinkV && insideLinkH;
		bool insideSlimEdgeLT      = insideLinkH && insideTopLeft;
		bool insideSlimEdgeTL      = insideLinkV && insideTopLeft;
		bool insideSlimEdgeTR      = insideLinkV && insideTopRight;
		bool insideSlimEdgeRT      = insideLinkH && insideTopRight;
		bool insideSlimEdgeRB      = insideLinkH && insideBottomRight;
		bool insideSlimEdgeBR      = insideLinkV && insideBottomRight;
		bool insideSlimEdgeBL      = insideLinkV && insideBottomLeft;
		bool insideSlimEdgeLB      = insideLinkH && insideBottomLeft;
		bool insideCenterLinkV     = insideLinkV && insideCircle && !insideBothLinks;
		bool insideCenterLinkH     = insideLinkH && insideCircle && !insideBothLinks;
		bool insideFreeCircle      = insideCircle      && !insideLinkV && !insideLinkH;
		bool insideFreeTopLeft     = insideTopLeft     && !insideLinkV && !insideLinkH;
		bool insideFreeTopRight    = insideTopRight    && !insideLinkV && !insideLinkH;
		bool insideFreeBottomRight = insideBottomRight && !insideLinkV && !insideLinkH;
		bool insideFreeBottomLeft  = insideBottomLeft  && !insideLinkV && !insideLinkH;

		uint leftPartValue         = (cellNumber.x > 0                                              ) * CellValue(cellNumber + int2(-1,  0));
		uint rightPartValue        = (cellNumber.x < iBoardSize - 1                                 ) * CellValue(cellNumber + int2( 1,  0));
		uint topPartValue          = (                                 cellNumber.y > 0             ) * CellValue(cellNumber + int2( 0, -1));
		uint bottomPartValue       = (                                 cellNumber.y < iBoardSize - 1) * CellValue(cellNumber + int2( 0,  1));
		uint leftTopPartValue      = (cellNumber.x > 0              && cellNumber.y > 0             ) * CellValue(cellNumber + int2(-1, -1));
		uint rightTopPartValue     = (cellNumber.x < iBoardSize - 1 && cellNumber.y > 0             ) * CellValue(cellNumber + int2( 1, -1));
		uint leftBottomPartValue   = (cellNumber.y < iBoardSize - 1 && cellNumber.x > 0             ) * CellValue(cellNumber + int2(-1,  1));
		uint rightBottomPartValue  = (cellNumber.x < iBoardSize - 1 && cellNumber.y < iBoardSize - 1) * CellValue(cellNumber + int2( 1,  1));
		uint left2PartValue        = (cellNumber.x > 1                                              ) * CellValue(cellNumber + int2(-2,  0));
		uint right2PartValue       = (cellNumber.x < iBoardSize - 2                                 ) * CellValue(cellNumber + int2( 2,  0));
		uint top2PartValue         = (                                 cellNumber.y > 1             ) * CellValue(cellNumber + int2( 0, -2));
		uint bottom2PartValue      = (                                 cellNumber.y < iBoardSize - 2) * CellValue(cellNumber + int2( 0,  2));

		uint CenterCandidate = cellValue;

		uint LTCandidate = (leftPartValue   == topPartValue   ) * leftPartValue;
		uint RTCandidate = (topPartValue    == rightPartValue ) * topPartValue;
		uint RBCandidate = (rightPartValue  == bottomPartValue) * rightPartValue;
		uint LBCandidate = (bottomPartValue == leftPartValue  ) * bottomPartValue;

		uint LTCCandidate = (cellValue == leftTopPartValue     || cellValue == topPartValue    || cellValue == leftPartValue)   * cellValue;
		uint RTCCandidate = (cellValue == rightTopPartValue    || cellValue == rightPartValue  || cellValue == topPartValue)    * cellValue;
		uint RBCCandidate = (cellValue == rightBottomPartValue || cellValue == bottomPartValue || cellValue == rightPartValue)  * cellValue;
		uint LBCCandidate = (cellValue == leftBottomPartValue  || cellValue == leftPartValue   || cellValue == bottomPartValue) * cellValue;

		uint VLCandidate = (topPartValue  == bottomPartValue) * topPartValue;
		uint HLCandidate = (leftPartValue == rightPartValue)  * leftPartValue;

		uint LSECandidate = (cellValue == left2PartValue)   * cellValue;
		uint TSECandidate = (cellValue == top2PartValue)    * cellValue;
		uint RSECandidate = (cellValue == right2PartValue)  * cellValue;
		uint BSECandidate = (cellValue == bottom2PartValue) * cellValue;

		uint resLT = max(LTCandidate, LTCCandidate);
		uint resRT = max(RTCandidate, RTCCandidate);
		uint resRB = max(RBCandidate, RBCCandidate);
		uint resLB = max(LBCandidate, LBCCandidate);

		uint resSLT = max(resLT, LSECandidate);
		uint resSTL = max(resLT, TSECandidate);
		uint resSTR = max(resRT, TSECandidate);
		uint resSRT = max(resRT, RSECandidate);
		uint resSRB = max(resRB, RSECandidate);
		uint resSBR = max(resRB, BSECandidate);
		uint resSBL = max(resLB, BSECandidate);
		uint resSLB = max(resLB, LSECandidate);

		uint resVL = max(VLCandidate, CenterCandidate);
		uint resHL = max(HLCandidate, CenterCandidate);

		uint resM = max(resVL, resHL);

		float cellPower = (float)cellValue / (gDomainSize - 1.0f);
		float LTPower   = (float)resLT     / (gDomainSize - 1.0f);
		float RTPower   = (float)resRT     / (gDomainSize - 1.0f);
		float RBPower   = (float)resRB     / (gDomainSize - 1.0f);
		float LBPower   = (float)resLB     / (gDomainSize - 1.0f);
		float SLTPower  = (float)resSLT    / (gDomainSize - 1.0f);
		float STLPower  = (float)resSTL    / (gDomainSize - 1.0f);
		float STRPower  = (float)resSTR    / (gDomainSize - 1.0f);
		float SRTPower  = (float)resSRT    / (gDomainSize - 1.0f);
		float SRBPower  = (float)resSRB    / (gDomainSize - 1.0f);
		float SBRPower  = (float)resSBR    / (gDomainSize - 1.0f);
		float SBLPower  = (float)resSBL    / (gDomainSize - 1.0f);
		float SLBPower  = (float)resSLB    / (gDomainSize - 1.0f);
		float VLPower   = (float)resVL     / (gDomainSize - 1.0f);
		float HLPower   = (float)resHL     / (gDomainSize - 1.0f);
		float MPower    = (float)resM      / (gDomainSize - 1.0f);

		float enablePower = cellPower * insideFreeCircle + insideFreeTopLeft * LTPower + insideFreeTopRight * RTPower + insideFreeBottomRight * RBPower + insideFreeBottomLeft * LBPower 
			              + insideCenterLinkV * VLPower + insideCenterLinkH * HLPower + insideBothLinks * MPower
			              + insideSlimEdgeLT * SLTPower + insideSlimEdgeTL * STLPower + insideSlimEdgeTR * STRPower + insideSlimEdgeRT * SRTPower
			              + insideSlimEdgeRB * SRBPower + insideSlimEdgeBR * SBRPower + insideSlimEdgeBL * SBLPower + insideSlimEdgeLB * SLBPower;

		result = lerp(gColorNone, gColorEnabled, enablePower);

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
			uint left2PartSolved       = (cellNumber.x > 1                                               ) * CellValueSolution(cellNumber + int2(-2,  0));
			uint right2PartSolved      = (cellNumber.x < iBoardSize - 2                                  ) * CellValueSolution(cellNumber + int2( 2,  0));
			uint top2PartSolved        = (                                 cellNumber.y > 1              ) * CellValueSolution(cellNumber + int2( 0, -2));
			uint bottom2PartSolved     = (                                 cellNumber.y < iBoardSize - 2 ) * CellValueSolution(cellNumber + int2( 0,  2));

			uint CenterSolutionCandidate = solutionValue;

			uint LTSolutionCandidate = (leftPartSolved   == topPartSolved   ) * leftPartSolved;
			uint RTSolutionCandidate = (topPartSolved    == rightPartSolved ) * topPartSolved;
			uint RBSolutionCandidate = (rightPartSolved  == bottomPartSolved) * rightPartSolved;
			uint LBSolutionCandidate = (bottomPartSolved == leftPartSolved  ) * bottomPartSolved;

			uint LTCSolutionCandidate = (solutionValue == leftTopPartSolved     || solutionValue == topPartSolved    || solutionValue == leftPartSolved)   * solutionValue;
			uint RTCSolutionCandidate = (solutionValue == rightTopPartSolved    || solutionValue == rightPartSolved  || solutionValue == topPartSolved)    * solutionValue;
			uint RBCSolutionCandidate = (solutionValue == rightBottomPartSolved || solutionValue == bottomPartSolved || solutionValue == rightPartSolved)  * solutionValue;
			uint LBCSolutionCandidate = (solutionValue == leftBottomPartSolved  || solutionValue == leftPartSolved   || solutionValue == bottomPartSolved) * solutionValue;

			uint VLSolutionCandidate = (topPartSolved  == bottomPartSolved) * topPartSolved;
			uint HLSolutionCandidate = (leftPartSolved == rightPartSolved)  * leftPartSolved;

			uint LSESolutionCandidate = (solutionValue == left2PartSolved)   * solutionValue;
			uint TSESolutionCandidate = (solutionValue == top2PartSolved)    * solutionValue;
			uint RSESolutionCandidate = (solutionValue == right2PartSolved)  * solutionValue;
			uint BSESolutionCandidate = (solutionValue == bottom2PartSolved) * solutionValue;

			uint resLTSolution = max(LTSolutionCandidate, LTCSolutionCandidate);
			uint resRTSolution = max(RTSolutionCandidate, RTCSolutionCandidate);
			uint resRBSolution = max(RBSolutionCandidate, RBCSolutionCandidate);
			uint resLBSolution = max(LBSolutionCandidate, LBCSolutionCandidate);

			uint resSLTSolution = max(resLTSolution, LSESolutionCandidate);
			uint resSTLSolution = max(resLTSolution, TSESolutionCandidate);
			uint resSTRSolution = max(resRTSolution, TSESolutionCandidate);
			uint resSRTSolution = max(resRTSolution, RSESolutionCandidate);
			uint resSRBSolution = max(resRBSolution, RSESolutionCandidate);
			uint resSBRSolution = max(resRBSolution, BSESolutionCandidate);
			uint resSBLSolution = max(resLBSolution, BSESolutionCandidate);
			uint resSLBSolution = max(resLBSolution, LSESolutionCandidate);

			uint resVLSolution = max(VLSolutionCandidate, CenterSolutionCandidate);
			uint resHLSolution = max(HLSolutionCandidate, CenterSolutionCandidate);

			uint resMSolution = max(resVLSolution, resHLSolution);

			float solutionPower    = (float)solutionValue  / (gDomainSize - 1.0f);
			float LTsolutionPower  = (float)resLTSolution  / (gDomainSize - 1.0f);
			float RTsolutionPower  = (float)resRTSolution  / (gDomainSize - 1.0f);
			float RBsolutionPower  = (float)resRBSolution  / (gDomainSize - 1.0f);
			float LBsolutionPower  = (float)resLBSolution  / (gDomainSize - 1.0f);
			float SLTsolutionPower = (float)resSLTSolution / (gDomainSize - 1.0f);
			float STLsolutionPower = (float)resSTLSolution / (gDomainSize - 1.0f);
			float STRsolutionPower = (float)resSTRSolution / (gDomainSize - 1.0f);
			float SRTsolutionPower = (float)resSRTSolution / (gDomainSize - 1.0f);
			float SRBsolutionPower = (float)resSRBSolution / (gDomainSize - 1.0f);
			float SBRsolutionPower = (float)resSBRSolution / (gDomainSize - 1.0f);
			float SBLsolutionPower = (float)resSBLSolution / (gDomainSize - 1.0f);
			float SLBsolutionPower = (float)resSLBSolution / (gDomainSize - 1.0f);
			float VLsolutionPower  = (float)resVLSolution  / (gDomainSize - 1.0f);
			float HLsolutionPower  = (float)resHLSolution  / (gDomainSize - 1.0f);
			float MsolutionPower   = (float)resMSolution   / (gDomainSize - 1.0f);

			float solvedPower = insideFreeCircle * solutionPower + insideFreeTopLeft * LTsolutionPower + insideFreeTopRight * RTsolutionPower + insideFreeBottomRight * RBsolutionPower + insideFreeBottomLeft * LBsolutionPower
							  + insideCenterLinkV * VLsolutionPower + insideCenterLinkH * HLsolutionPower + insideBothLinks * MsolutionPower
							  + insideSlimEdgeLT * SLTsolutionPower + insideSlimEdgeTL * STLsolutionPower + insideSlimEdgeTR * STRsolutionPower + insideSlimEdgeRT * SRTsolutionPower
							  + insideSlimEdgeRB * SRBsolutionPower + insideSlimEdgeBR * SBRsolutionPower + insideSlimEdgeBL * SBLsolutionPower + insideSlimEdgeLB * SLBsolutionPower;

			result = lerp(result, gColorSolved, solvedPower);
		}
		else if(gFlags & FLAG_SHOW_STABILITY)
		{
			uint   stabilityValue = CellValueStability(cellNumber);
			float4 colorStable    = float4(1.0f, 1.0f, 1.0f, 1.0f) - gColorEnabled;

			uint leftPartStable        = (cellNumber.x > 0             )                                   * CellValueStability(cellNumber + int2(-1,  0));
			uint rightPartStable       = (cellNumber.x < gBoardSize - 1)                                   * CellValueStability(cellNumber + int2( 1,  0));
			uint topPartStable         = (cellNumber.y > 0             )                                   * CellValueStability(cellNumber + int2( 0, -1));
			uint bottomPartStable      = (cellNumber.y < gBoardSize - 1)                                   * CellValueStability(cellNumber + int2( 0,  1));
			uint leftTopPartStable     = (cellNumber.x > 0             ) * (cellNumber.y > 0             ) * CellValueStability(cellNumber + int2(-1, -1));
			uint rightTopPartStable    = (cellNumber.x < gBoardSize - 1) * (cellNumber.y > 0             ) * CellValueStability(cellNumber + int2( 1, -1));
			uint leftBottomPartStable  = (cellNumber.y < gBoardSize - 1) * (cellNumber.x > 0             ) * CellValueStability(cellNumber + int2(-1,  1));
			uint rightBottomPartStable = (cellNumber.x < gBoardSize - 1) * (cellNumber.y < gBoardSize - 1) * CellValueStability(cellNumber + int2( 1,  1));
			uint left2PartStable       = (cellNumber.x > 1                                               ) * CellValueStability(cellNumber + int2(-2,  0));
			uint right2PartStable      = (cellNumber.x < iBoardSize - 2                                  ) * CellValueStability(cellNumber + int2( 2,  0));
			uint top2PartStable        = (                                 cellNumber.y > 1              ) * CellValueStability(cellNumber + int2( 0, -2));
			uint bottom2PartStable     = (                                 cellNumber.y < iBoardSize - 2 ) * CellValueStability(cellNumber + int2( 0,  2));

			uint CenterStabilityCandidate = stabilityValue;

			uint LTStabilityCandidate = (leftPartStable   == topPartStable   ) * leftPartStable;
			uint RTStabilityCandidate = (topPartStable    == rightPartStable ) * topPartStable;
			uint RBStabilityCandidate = (rightPartStable  == bottomPartStable) * rightPartStable;
			uint LBStabilityCandidate = (bottomPartStable == leftPartStable  ) * bottomPartStable;

			uint LTCStabilityCandidate = (stabilityValue == leftTopPartStable     || stabilityValue == topPartStable    || stabilityValue == leftPartStable)   * stabilityValue;
			uint RTCStabilityCandidate = (stabilityValue == rightTopPartStable    || stabilityValue == rightPartStable  || stabilityValue == topPartStable)    * stabilityValue;
			uint RBCStabilityCandidate = (stabilityValue == rightBottomPartStable || stabilityValue == bottomPartStable || stabilityValue == rightPartStable)  * stabilityValue;
			uint LBCStabilityCandidate = (stabilityValue == leftBottomPartStable  || stabilityValue == leftPartStable   || stabilityValue == bottomPartStable) * stabilityValue;

			uint VLStabilityCandidate = (topPartStable  == bottomPartStable) * topPartStable;
			uint HLStabilityCandidate = (leftPartStable == rightPartStable)  * leftPartStable;

			uint LSEStabilityCandidate = (stabilityValue == left2PartStable)   * stabilityValue;
			uint TSEStabilityCandidate = (stabilityValue == top2PartStable)    * stabilityValue;
			uint RSEStabilityCandidate = (stabilityValue == right2PartStable)  * stabilityValue;
			uint BSEStabilityCandidate = (stabilityValue == bottom2PartStable) * stabilityValue;

			uint resLTStability = max(LTStabilityCandidate, LTCStabilityCandidate);
			uint resRTStability = max(RTStabilityCandidate, RTCStabilityCandidate);
			uint resRBStability = max(RBStabilityCandidate, RBCStabilityCandidate);
			uint resLBStability = max(LBStabilityCandidate, LBCStabilityCandidate);

			uint resSLTStability = max(resLTStability, LSEStabilityCandidate);
			uint resSTLStability = max(resLTStability, TSEStabilityCandidate);
			uint resSTRStability = max(resRTStability, TSEStabilityCandidate);
			uint resSRTStability = max(resRTStability, RSEStabilityCandidate);
			uint resSRBStability = max(resRBStability, RSEStabilityCandidate);
			uint resSBRStability = max(resRBStability, BSEStabilityCandidate);
			uint resSBLStability = max(resLBStability, BSEStabilityCandidate);
			uint resSLBStability = max(resLBStability, LSEStabilityCandidate);

			uint resVLStability = max(VLStabilityCandidate, CenterStabilityCandidate);
			uint resHLStability = max(HLStabilityCandidate, CenterStabilityCandidate);

			uint resMStability = max(resVLStability, resHLStability);

			float stabilityPower    = (float)stabilityValue  / (gDomainSize - 1.0f);
			float LTstabilityPower  = (float)resLTStability  / (gDomainSize - 1.0f);
			float RTstabilityPower  = (float)resRTStability  / (gDomainSize - 1.0f);
			float RBstabilityPower  = (float)resRBStability  / (gDomainSize - 1.0f);
			float LBstabilityPower  = (float)resLBStability  / (gDomainSize - 1.0f);
			float SLTstabilityPower = (float)resSLTStability / (gDomainSize - 1.0f);
			float STLstabilityPower = (float)resSTLStability / (gDomainSize - 1.0f);
			float STRstabilityPower = (float)resSTRStability / (gDomainSize - 1.0f);
			float SRTstabilityPower = (float)resSRTStability / (gDomainSize - 1.0f);
			float SRBstabilityPower = (float)resSRBStability / (gDomainSize - 1.0f);
			float SBRstabilityPower = (float)resSBRStability / (gDomainSize - 1.0f);
			float SBLstabilityPower = (float)resSBLStability / (gDomainSize - 1.0f);
			float SLBstabilityPower = (float)resSLBStability / (gDomainSize - 1.0f);
			float VLstabilityPower  = (float)resVLStability  / (gDomainSize - 1.0f);
			float HLstabilityPower  = (float)resHLStability  / (gDomainSize - 1.0f);
			float MstabilityPower   = (float)resMStability   / (gDomainSize - 1.0f);

			float stablePower = insideFreeCircle * stabilityPower + insideFreeTopLeft * LTstabilityPower + insideFreeTopRight * RTstabilityPower + insideFreeBottomRight * RBstabilityPower + insideFreeBottomLeft * LBstabilityPower
							  + insideCenterLinkV * VLstabilityPower + insideCenterLinkH * HLstabilityPower + insideBothLinks * MstabilityPower
							  + insideSlimEdgeLT * SLTstabilityPower + insideSlimEdgeTL * STLstabilityPower + insideSlimEdgeTR * STRstabilityPower + insideSlimEdgeRT * SRTstabilityPower
							  + insideSlimEdgeRB * SRBstabilityPower + insideSlimEdgeBR * SBRstabilityPower + insideSlimEdgeBL * SBLstabilityPower + insideSlimEdgeLB * SLBstabilityPower;
			
			result = lerp(result, colorStable, stablePower);
		}
	}
	else //Edge of the cell
	{
		result = gColorBetween;
	}

	Result[DTid.xy] = result;
}