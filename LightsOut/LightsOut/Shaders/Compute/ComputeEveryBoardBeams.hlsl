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

		bool insideCentralDiamond = (abs(cellCoord.x) + abs(cellCoord.y) <= (gCellSize - 1) / 2);

		bool insideHorizontalBeamLeft  = (abs(cellCoord.y) <= 0.707f * (gCellSize - 1) / 2 && cellCoord.x < 0);
		bool insideHorizontalBeamRight = (abs(cellCoord.y) <= 0.707f * (gCellSize - 1) / 2 && cellCoord.x > 0);
		bool insideVerticalBeamTop     = (abs(cellCoord.x) <= 0.707f * (gCellSize - 1) / 2 && cellCoord.y < 0);
		bool insideVerticalBeamBottom  = (abs(cellCoord.x) <= 0.707f * (gCellSize - 1) / 2 && cellCoord.y > 0);

		bool insideCentralOctagon = insideCentralDiamond && (insideHorizontalBeamLeft || insideHorizontalBeamRight) && (insideVerticalBeamTop || insideVerticalBeamBottom);

		bool insideTopLeftBeamCorner     = insideVerticalBeamTop    && insideHorizontalBeamLeft  && !insideCentralDiamond;
		bool insideTopRightBeamCorner    = insideVerticalBeamTop    && insideHorizontalBeamRight && !insideCentralDiamond;
		bool insideBottomRightBeamCorner = insideVerticalBeamBottom && insideHorizontalBeamRight && !insideCentralDiamond;
		bool insideBottomLeftBeamCorner  = insideVerticalBeamBottom && insideHorizontalBeamLeft  && !insideCentralDiamond;

		bool insideTopBeamEnd    = insideVerticalBeamTop     && !insideHorizontalBeamLeft  && !insideHorizontalBeamRight && insideCentralDiamond;
		bool insideBottomBeamEnd = insideVerticalBeamBottom  && !insideHorizontalBeamRight && !insideHorizontalBeamLeft  && insideCentralDiamond;
		bool insideLeftBeamEnd   = insideHorizontalBeamLeft  && !insideVerticalBeamTop     && !insideVerticalBeamBottom  && insideCentralDiamond;
		bool insideRightBeamEnd  = insideHorizontalBeamRight && !insideVerticalBeamBottom  && !insideVerticalBeamTop     && insideCentralDiamond;

		bool insideTopLeftEdge     = insideVerticalBeamTop     && !insideHorizontalBeamLeft  && !insideCentralDiamond && cellCoord.x <= 0;
		bool insideTopRightEdge    = insideVerticalBeamTop     && !insideHorizontalBeamRight && !insideCentralDiamond && cellCoord.x >= 0;
		bool insideRightTopEdge    = insideHorizontalBeamRight && !insideVerticalBeamTop     && !insideCentralDiamond && cellCoord.y <= 0;
		bool insideRightBottomEdge = insideHorizontalBeamRight && !insideVerticalBeamBottom  && !insideCentralDiamond && cellCoord.y >= 0;
		bool insideBottomRightEdge = insideVerticalBeamBottom  && !insideHorizontalBeamRight && !insideCentralDiamond && cellCoord.x >= 0;
		bool insideBottomLeftEdge  = insideVerticalBeamBottom  && !insideHorizontalBeamLeft  && !insideCentralDiamond && cellCoord.x <= 0;
		bool insideLeftBottomEdge  = insideHorizontalBeamLeft  && !insideVerticalBeamBottom  && !insideCentralDiamond && cellCoord.y >= 0;
		bool insideLeftTopEdge     = insideHorizontalBeamLeft  && !insideVerticalBeamTop     && !insideCentralDiamond && cellCoord.y <= 0;

		bool insideTopLeftCorner     = !insideVerticalBeamTop    && !insideHorizontalBeamLeft  && cellCoord.x <= 0 && cellCoord.y <= 0;
		bool insideTopRightCorner    = !insideVerticalBeamTop    && !insideHorizontalBeamRight && cellCoord.x >= 0 && cellCoord.y <= 0;
		bool insideBottomRightCorner = !insideVerticalBeamBottom && !insideHorizontalBeamRight && cellCoord.x >= 0 && cellCoord.y >= 0;
		bool insideBottomLeftCorner  = !insideVerticalBeamBottom && !insideHorizontalBeamLeft  && cellCoord.x <= 0 && cellCoord.y >= 0;

		uint leftPartValue        = (cellNumber.x > 0             )                                   * CellValue(cellNumber + int2(-1,  0));
		uint rightPartValue       = (cellNumber.x < gBoardSize - 1)                                   * CellValue(cellNumber + int2( 1,  0));
		uint topPartValue         = (cellNumber.y > 0             )                                   * CellValue(cellNumber + int2( 0, -1));
		uint bottomPartValue      = (cellNumber.y < gBoardSize - 1)                                   * CellValue(cellNumber + int2( 0,  1));
		uint leftTopPartValue     = (cellNumber.x > 0             ) * (cellNumber.y > 0             ) * CellValue(cellNumber + int2(-1, -1));
		uint rightTopPartValue    = (cellNumber.x < gBoardSize - 1) * (cellNumber.y > 0             ) * CellValue(cellNumber + int2( 1, -1));
		uint leftBottomPartValue  = (cellNumber.y < gBoardSize - 1) * (cellNumber.x > 0             ) * CellValue(cellNumber + int2(-1,  1));
		uint rightBottomPartValue = (cellNumber.x < gBoardSize - 1) * (cellNumber.y < gBoardSize - 1) * CellValue(cellNumber + int2( 1,  1));
		
		uint loneValue = cellValue != leftPartValue && cellValue != rightPartValue && cellValue != topPartValue && cellValue != bottomPartValue && cellValue != leftTopPartValue && cellValue != rightTopPartValue && cellValue != leftBottomPartValue && cellValue != rightBottomPartValue;

		uint LTCCandidate = (leftPartValue   == topPartValue    && leftPartValue   != leftTopPartValue     && leftPartValue   != cellValue) * leftPartValue;
		uint RTCCandidate = (topPartValue    == rightPartValue  && topPartValue    != rightTopPartValue    && topPartValue    != cellValue) * topPartValue;
		uint RBCCandidate = (rightPartValue  == bottomPartValue && rightPartValue  != rightBottomPartValue && rightPartValue  != cellValue) * rightPartValue;
		uint LBCCandidate = (bottomPartValue == leftPartValue   && bottomPartValue != leftBottomPartValue  && bottomPartValue != cellValue) * bottomPartValue;

		uint BLTCandidate = ((cellValue == topPartValue)    || (cellValue == leftPartValue)   || (cellValue == leftTopPartValue)     || (cellValue == rightPartValue  && cellValue == bottomPartValue) || (cellValue == rightBottomPartValue && cellValue != leftBottomPartValue  && cellValue != bottomPartValue && cellValue != rightPartValue  && cellValue != rightTopPartValue)   ) * cellValue;
		uint BRTCandidate = ((cellValue == rightPartValue)  || (cellValue == topPartValue)    || (cellValue == rightTopPartValue)    || (cellValue == bottomPartValue && cellValue == leftPartValue)   || (cellValue == leftBottomPartValue  && cellValue != leftTopPartValue     && cellValue != leftPartValue   && cellValue != bottomPartValue && cellValue != rightBottomPartValue)) * cellValue;
		uint BRBCandidate = ((cellValue == bottomPartValue) || (cellValue == rightPartValue)  || (cellValue == rightBottomPartValue) || (cellValue == leftPartValue   && cellValue == topPartValue)    || (cellValue == leftTopPartValue     && cellValue != rightTopPartValue    && cellValue != topPartValue    && cellValue != leftPartValue   && cellValue != leftBottomPartValue) ) * cellValue;
		uint BLBCandidate = ((cellValue == leftPartValue)   || (cellValue == bottomPartValue) || (cellValue == leftBottomPartValue)  || (cellValue == topPartValue    && cellValue == rightPartValue)  || (cellValue == rightTopPartValue    && cellValue != rightBottomPartValue && cellValue != rightPartValue  && cellValue != topPartValue    && cellValue != leftTopPartValue)    ) * cellValue;

		uint BTCandidate = ((cellValue == topPartValue)    || (cellValue == leftTopPartValue     && cellValue != leftPartValue)   || (cellValue == rightTopPartValue    && cellValue != rightPartValue)  || (cellValue == leftBottomPartValue  && cellValue == rightBottomPartValue && cellValue != leftPartValue   && cellValue != rightPartValue)  || (cellValue == bottomPartValue && cellValue != leftPartValue   && cellValue != rightPartValue)  || loneValue) * cellValue;
		uint BRCandidate = ((cellValue == rightPartValue)  || (cellValue == rightTopPartValue    && cellValue != topPartValue)    || (cellValue == rightBottomPartValue && cellValue != bottomPartValue) || (cellValue == leftTopPartValue     && cellValue == leftBottomPartValue  && cellValue != topPartValue    && cellValue != bottomPartValue) || (cellValue == leftPartValue   && cellValue != topPartValue    && cellValue != bottomPartValue) || loneValue) * cellValue;
		uint BBCandidate = ((cellValue == bottomPartValue) || (cellValue == rightBottomPartValue && cellValue != rightPartValue)  || (cellValue == leftBottomPartValue  && cellValue != leftPartValue)   || (cellValue == rightTopPartValue    && cellValue == leftTopPartValue     && cellValue != rightPartValue  && cellValue != leftPartValue)   || (cellValue == topPartValue    && cellValue != rightPartValue  && cellValue != leftPartValue)   || loneValue) * cellValue;
		uint BLCandidate = ((cellValue == leftPartValue)   || (cellValue == leftBottomPartValue  && cellValue != bottomPartValue) || (cellValue == leftTopPartValue     && cellValue != topPartValue)    || (cellValue == rightBottomPartValue && cellValue == rightTopPartValue    && cellValue != bottomPartValue && cellValue != topPartValue)    || (cellValue == rightPartValue  && cellValue != bottomPartValue && cellValue != topPartValue)    || loneValue) * cellValue;

		uint ETLCandidate = ((cellValue == topPartValue)    || (cellValue == leftTopPartValue     && cellValue != leftPartValue))   * cellValue;
		uint ETRCandidate = ((cellValue == topPartValue)    || (cellValue == rightTopPartValue    && cellValue != rightPartValue))  * cellValue;
		uint ERTCandidate = ((cellValue == rightPartValue)  || (cellValue == rightTopPartValue    && cellValue != topPartValue))    * cellValue;
		uint ERBCandidate = ((cellValue == rightPartValue)  || (cellValue == rightBottomPartValue && cellValue != bottomPartValue)) * cellValue;
		uint EBRCandidate = ((cellValue == bottomPartValue) || (cellValue == rightBottomPartValue && cellValue != rightPartValue))  * cellValue;
		uint EBLCandidate = ((cellValue == bottomPartValue) || (cellValue == leftBottomPartValue  && cellValue != leftPartValue))   * cellValue;
		uint ELBCandidate = ((cellValue == leftPartValue)   || (cellValue == leftBottomPartValue  && cellValue != bottomPartValue)) * cellValue;
		uint ELTCandidate = ((cellValue == leftPartValue)   || (cellValue == leftTopPartValue     && cellValue != topPartValue))    * cellValue;

		uint CLTCandidate = (cellValue == leftTopPartValue     && cellValue != topPartValue    && cellValue != leftPartValue)   * cellValue;
		uint CRTCandidate = (cellValue == rightTopPartValue    && cellValue != rightPartValue  && cellValue != topPartValue)    * cellValue;
		uint CRBCandidate = (cellValue == rightBottomPartValue && cellValue != bottomPartValue && cellValue != rightPartValue)  * cellValue;
		uint CLBCandidate = (cellValue == leftBottomPartValue  && cellValue != leftPartValue   && cellValue != bottomPartValue) * cellValue;

		uint resBLT = max(BLTCandidate, LTCCandidate);
		uint resBRT = max(BRTCandidate, RTCCandidate);
		uint resBRB = max(BRBCandidate, RBCCandidate);
		uint resBLB = max(BLBCandidate, LBCCandidate);

		uint resETL = max(ETLCandidate, LTCCandidate);
		uint resETR = max(ETRCandidate, RTCCandidate);
		uint resERT = max(ERTCandidate, RTCCandidate);
		uint resERB = max(ERBCandidate, RBCCandidate);
		uint resEBR = max(EBRCandidate, RBCCandidate);
		uint resEBL = max(EBLCandidate, LBCCandidate);
		uint resELB = max(ELBCandidate, LBCCandidate);
		uint resELT = max(ELTCandidate, LTCCandidate);

		uint resCLT = max(CLTCandidate, LTCCandidate);
		uint resCRT = max(CRTCandidate, RTCCandidate);
		uint resCRB = max(CRBCandidate, RBCCandidate);
		uint resCLB = max(CLBCandidate, LBCCandidate);

		float cellPower = (float)cellValue   / (gDomainSize - 1.0f);
		float BLTPower  = (float)resBLT      / (gDomainSize - 1.0f);
		float BRTPower  = (float)resBRT      / (gDomainSize - 1.0f);
		float BRBPower  = (float)resBRB      / (gDomainSize - 1.0f);
		float BLBPower  = (float)resBLB      / (gDomainSize - 1.0f);
		float BTPower   = (float)BTCandidate / (gDomainSize - 1.0f);
		float BRPower   = (float)BRCandidate / (gDomainSize - 1.0f);
		float BBPower   = (float)BBCandidate / (gDomainSize - 1.0f);
		float BLPower   = (float)BLCandidate / (gDomainSize - 1.0f);
		float ETLPower  = (float)resETL      / (gDomainSize - 1.0f);
		float ETRPower  = (float)resETR      / (gDomainSize - 1.0f);
		float ERTPower  = (float)resERT      / (gDomainSize - 1.0f);
		float ERBPower  = (float)resERB      / (gDomainSize - 1.0f);
		float EBRPower  = (float)resEBR      / (gDomainSize - 1.0f);
		float EBLPower  = (float)resEBL      / (gDomainSize - 1.0f);
		float ELBPower  = (float)resELB      / (gDomainSize - 1.0f);
		float ELTPower  = (float)resELT      / (gDomainSize - 1.0f);
		float CLTPower  = (float)resCLT      / (gDomainSize - 1.0f);
		float CRTPower  = (float)resCRT      / (gDomainSize - 1.0f);
		float CRBPower  = (float)resCRB      / (gDomainSize - 1.0f);
		float CLBPower  = (float)resCLB      / (gDomainSize - 1.0f);

		float enablePower = cellPower * insideCentralOctagon + BLTPower * insideTopLeftBeamCorner + BRTPower * insideTopRightBeamCorner + BRBPower * insideBottomRightBeamCorner + BLBPower * insideBottomLeftBeamCorner + BTPower * insideTopBeamEnd + BRPower * insideRightBeamEnd + BBPower * insideBottomBeamEnd + BLPower * insideLeftBeamEnd + ETLPower * insideTopLeftEdge + ETRPower * insideTopRightEdge + ERTPower * insideRightTopEdge + ERBPower * insideRightBottomEdge + EBRPower * insideBottomRightEdge + EBLPower * insideBottomLeftEdge + ELBPower * insideLeftBottomEdge + ELTPower * insideLeftTopEdge + CLTPower * insideTopLeftCorner + CRTPower * insideTopRightCorner + CRBPower * insideBottomRightCorner + CLBPower * insideBottomLeftCorner;
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

			uint loneSolved = solutionValue != leftPartSolved && solutionValue != rightPartSolved && solutionValue != topPartSolved && solutionValue != bottomPartSolved && solutionValue != leftTopPartSolved && solutionValue != rightTopPartSolved && solutionValue != leftBottomPartSolved && solutionValue != rightBottomPartSolved;

			uint LTCSolutionCandidate = (leftPartSolved   == topPartSolved    && leftPartSolved   != leftTopPartSolved     && leftPartSolved   != solutionValue) * leftPartSolved;
			uint RTCSolutionCandidate = (topPartSolved    == rightPartSolved  && topPartSolved    != rightTopPartSolved    && topPartSolved    != solutionValue) * topPartSolved;
			uint RBCSolutionCandidate = (rightPartSolved  == bottomPartSolved && rightPartSolved  != rightBottomPartSolved && rightPartSolved  != solutionValue) * rightPartSolved;
			uint LBCSolutionCandidate = (bottomPartSolved == leftPartSolved   && bottomPartSolved != leftBottomPartSolved  && bottomPartSolved != solutionValue) * bottomPartSolved;

			uint BLTSolutionCandidate = ((solutionValue == topPartSolved)    || (solutionValue == leftPartSolved)   || (solutionValue == leftTopPartSolved)     || (solutionValue == rightPartSolved  && solutionValue == bottomPartSolved) || (solutionValue == rightBottomPartSolved && solutionValue != leftBottomPartSolved  && solutionValue != bottomPartSolved && solutionValue != rightPartSolved  && solutionValue != rightTopPartSolved)   ) * solutionValue;
			uint BRTSolutionCandidate = ((solutionValue == rightPartSolved)  || (solutionValue == topPartSolved)    || (solutionValue == rightTopPartSolved)    || (solutionValue == bottomPartSolved && solutionValue == leftPartSolved)   || (solutionValue == leftBottomPartSolved  && solutionValue != leftTopPartSolved     && solutionValue != leftPartSolved   && solutionValue != bottomPartSolved && solutionValue != rightBottomPartSolved)) * solutionValue;
			uint BRBSolutionCandidate = ((solutionValue == bottomPartSolved) || (solutionValue == rightPartSolved)  || (solutionValue == rightBottomPartSolved) || (solutionValue == leftPartSolved   && solutionValue == topPartSolved)    || (solutionValue == leftTopPartSolved     && solutionValue != rightTopPartSolved    && solutionValue != topPartSolved    && solutionValue != leftPartSolved   && solutionValue != leftBottomPartSolved) ) * solutionValue;
			uint BLBSolutionCandidate = ((solutionValue == leftPartSolved)   || (solutionValue == bottomPartSolved) || (solutionValue == leftBottomPartSolved)  || (solutionValue == topPartSolved    && solutionValue == rightPartSolved)  || (solutionValue == rightTopPartSolved    && solutionValue != rightBottomPartSolved && solutionValue != rightPartSolved  && solutionValue != topPartSolved    && solutionValue != leftTopPartSolved)    ) * solutionValue;

			uint BTSolutionCandidate = ((solutionValue == topPartSolved)    || (solutionValue == leftTopPartSolved     && solutionValue != leftPartSolved)   || (solutionValue == rightTopPartSolved    && solutionValue != rightPartSolved)  || (solutionValue == leftBottomPartSolved  && solutionValue == rightBottomPartSolved && solutionValue != leftPartSolved   && solutionValue != rightPartSolved)  || (solutionValue == bottomPartSolved && solutionValue != leftPartSolved   && solutionValue != rightPartSolved)  || loneSolved) * solutionValue;
			uint BRSolutionCandidate = ((solutionValue == rightPartSolved)  || (solutionValue == rightTopPartSolved    && solutionValue != topPartSolved)    || (solutionValue == rightBottomPartSolved && solutionValue != bottomPartSolved) || (solutionValue == leftTopPartSolved     && solutionValue == leftBottomPartSolved  && solutionValue != topPartSolved    && solutionValue != bottomPartSolved) || (solutionValue == leftPartSolved   && solutionValue != topPartSolved    && solutionValue != bottomPartSolved) || loneSolved) * solutionValue;
			uint BBSolutionCandidate = ((solutionValue == bottomPartSolved) || (solutionValue == rightBottomPartSolved && solutionValue != rightPartSolved)  || (solutionValue == leftBottomPartSolved  && solutionValue != leftPartSolved)   || (solutionValue == rightTopPartSolved    && solutionValue == leftTopPartSolved     && solutionValue != rightPartSolved  && solutionValue != leftPartSolved)   || (solutionValue == topPartSolved    && solutionValue != rightPartSolved  && solutionValue != leftPartSolved)   || loneSolved) * solutionValue;
			uint BLSolutionCandidate = ((solutionValue == leftPartSolved)   || (solutionValue == leftBottomPartSolved  && solutionValue != bottomPartSolved) || (solutionValue == leftTopPartSolved     && solutionValue != topPartSolved)    || (solutionValue == rightBottomPartSolved && solutionValue == rightTopPartSolved    && solutionValue != bottomPartSolved && solutionValue != topPartSolved)    || (solutionValue == rightPartSolved  && solutionValue != bottomPartSolved && solutionValue != topPartSolved)    || loneSolved) * solutionValue;

			uint ETLSolutionCandidate = ((solutionValue == topPartSolved)    || (solutionValue == leftTopPartSolved     && solutionValue != leftPartSolved))   * solutionValue;
			uint ETRSolutionCandidate = ((solutionValue == topPartSolved)    || (solutionValue == rightTopPartSolved    && solutionValue != rightPartSolved))  * solutionValue;
			uint ERTSolutionCandidate = ((solutionValue == rightPartSolved)  || (solutionValue == rightTopPartSolved    && solutionValue != topPartSolved))    * solutionValue;
			uint ERBSolutionCandidate = ((solutionValue == rightPartSolved)  || (solutionValue == rightBottomPartSolved && solutionValue != bottomPartSolved)) * solutionValue;
			uint EBRSolutionCandidate = ((solutionValue == bottomPartSolved) || (solutionValue == rightBottomPartSolved && solutionValue != rightPartSolved))  * solutionValue;
			uint EBLSolutionCandidate = ((solutionValue == bottomPartSolved) || (solutionValue == leftBottomPartSolved  && solutionValue != leftPartSolved))   * solutionValue;
			uint ELBSolutionCandidate = ((solutionValue == leftPartSolved)   || (solutionValue == leftBottomPartSolved  && solutionValue != bottomPartSolved)) * solutionValue;
			uint ELTSolutionCandidate = ((solutionValue == leftPartSolved)   || (solutionValue == leftTopPartSolved     && solutionValue != topPartSolved))    * solutionValue;

			uint CLTSolutionCandidate = (solutionValue == leftTopPartSolved     && solutionValue != topPartSolved    && solutionValue != leftPartSolved)   * solutionValue;
			uint CRTSolutionCandidate = (solutionValue == rightTopPartSolved    && solutionValue != rightPartSolved  && solutionValue != topPartSolved)    * solutionValue;
			uint CRBSolutionCandidate = (solutionValue == rightBottomPartSolved && solutionValue != bottomPartSolved && solutionValue != rightPartSolved)  * solutionValue;
			uint CLBSolutionCandidate = (solutionValue == leftBottomPartSolved  && solutionValue != leftPartSolved   && solutionValue != bottomPartSolved) * solutionValue;

			uint resBLTSolved = max(BLTSolutionCandidate, LTCSolutionCandidate);
			uint resBRTSolved = max(BRTSolutionCandidate, RTCSolutionCandidate);
			uint resBRBSolved = max(BRBSolutionCandidate, RBCSolutionCandidate);
			uint resBLBSolved = max(BLBSolutionCandidate, LBCSolutionCandidate);

			uint resETLSolved = max(ETLSolutionCandidate, LTCSolutionCandidate);
			uint resETRSolved = max(ETRSolutionCandidate, RTCSolutionCandidate);
			uint resERTSolved = max(ERTSolutionCandidate, RTCSolutionCandidate);
			uint resERBSolved = max(ERBSolutionCandidate, RBCSolutionCandidate);
			uint resEBRSolved = max(EBRSolutionCandidate, RBCSolutionCandidate);
			uint resEBLSolved = max(EBLSolutionCandidate, LBCSolutionCandidate);
			uint resELBSolved = max(ELBSolutionCandidate, LBCSolutionCandidate);
			uint resELTSolved = max(ELTSolutionCandidate, LTCSolutionCandidate);

			uint resCLTSolved = max(CLTSolutionCandidate, LTCSolutionCandidate);
			uint resCRTSolved = max(CRTSolutionCandidate, RTCSolutionCandidate);
			uint resCRBSolved = max(CRBSolutionCandidate, RBCSolutionCandidate);
			uint resCLBSolved = max(CLBSolutionCandidate, LBCSolutionCandidate);

			float cellSolutionPower = (float)solutionValue       / (gDomainSize - 1.0f);
			float BLTSolutionPower  = (float)resBLTSolved        / (gDomainSize - 1.0f);
			float BRTSolutionPower  = (float)resBRTSolved        / (gDomainSize - 1.0f);
			float BRBSolutionPower  = (float)resBRBSolved        / (gDomainSize - 1.0f);
			float BLBSolutionPower  = (float)resBLBSolved        / (gDomainSize - 1.0f);
			float BTSolutionPower   = (float)BTSolutionCandidate / (gDomainSize - 1.0f);
			float BRSolutionPower   = (float)BRSolutionCandidate / (gDomainSize - 1.0f);
			float BBSolutionPower   = (float)BBSolutionCandidate / (gDomainSize - 1.0f);
			float BLSolutionPower   = (float)BLSolutionCandidate / (gDomainSize - 1.0f);
			float ETLSolutionPower  = (float)resETLSolved        / (gDomainSize - 1.0f);
			float ETRSolutionPower  = (float)resETRSolved        / (gDomainSize - 1.0f);
			float ERTSolutionPower  = (float)resERTSolved        / (gDomainSize - 1.0f);
			float ERBSolutionPower  = (float)resERBSolved        / (gDomainSize - 1.0f);
			float EBRSolutionPower  = (float)resEBRSolved        / (gDomainSize - 1.0f);
			float EBLSolutionPower  = (float)resEBLSolved        / (gDomainSize - 1.0f);
			float ELBSolutionPower  = (float)resELBSolved        / (gDomainSize - 1.0f);
			float ELTSolutionPower  = (float)resELTSolved        / (gDomainSize - 1.0f);
			float CLTSolutionPower  = (float)resCLTSolved        / (gDomainSize - 1.0f);
			float CRTSolutionPower  = (float)resCRTSolved        / (gDomainSize - 1.0f);
			float CRBSolutionPower  = (float)resCRBSolved        / (gDomainSize - 1.0f);
			float CLBSolutionPower  = (float)resCLBSolved        / (gDomainSize - 1.0f);

			float solutionPower = cellSolutionPower * insideCentralOctagon + BLTSolutionPower * insideTopLeftBeamCorner + BRTSolutionPower * insideTopRightBeamCorner + BRBSolutionPower * insideBottomRightBeamCorner + BLBSolutionPower * insideBottomLeftBeamCorner + BTSolutionPower * insideTopBeamEnd + BRSolutionPower * insideRightBeamEnd + BBSolutionPower * insideBottomBeamEnd + BLSolutionPower * insideLeftBeamEnd + ETLSolutionPower * insideTopLeftEdge + ETRSolutionPower * insideTopRightEdge + ERTSolutionPower * insideRightTopEdge + ERBSolutionPower * insideRightBottomEdge + EBRSolutionPower * insideBottomRightEdge + EBLSolutionPower * insideBottomLeftEdge + ELBSolutionPower * insideLeftBottomEdge + ELTSolutionPower * insideLeftTopEdge + CLTSolutionPower * insideTopLeftCorner + CRTSolutionPower * insideTopRightCorner + CRBSolutionPower * insideBottomRightCorner + CLBSolutionPower * insideBottomLeftCorner;
			result              = lerp(result, gColorSolved, solutionPower);
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

			uint loneStable = stableValue != leftPartStable && stableValue != rightPartStable && stableValue != topPartStable && stableValue != bottomPartStable && stableValue != leftTopPartStable && stableValue != rightTopPartStable && stableValue != leftBottomPartStable && stableValue != rightBottomPartStable;

			uint LTCStabilityCandidate = (leftPartStable   == topPartStable    && leftPartStable   != leftTopPartStable     && leftPartStable   != stableValue) * leftPartStable;
			uint RTCStabilityCandidate = (topPartStable    == rightPartStable  && topPartStable    != rightTopPartStable    && topPartStable    != stableValue) * topPartStable;
			uint RBCStabilityCandidate = (rightPartStable  == bottomPartStable && rightPartStable  != rightBottomPartStable && rightPartStable  != stableValue) * rightPartStable;
			uint LBCStabilityCandidate = (bottomPartStable == leftPartStable   && bottomPartStable != leftBottomPartStable  && bottomPartStable != stableValue) * bottomPartStable;

			uint BLTStabilityCandidate = ((stableValue == topPartStable)    || (stableValue == leftPartStable)   || (stableValue == leftTopPartStable)     || (stableValue == rightPartStable  && stableValue == bottomPartStable) || (stableValue == rightBottomPartStable && stableValue != leftBottomPartStable  && stableValue != bottomPartStable && stableValue != rightPartStable  && stableValue != rightTopPartStable)   ) * stableValue;
			uint BRTStabilityCandidate = ((stableValue == rightPartStable)  || (stableValue == topPartStable)    || (stableValue == rightTopPartStable)    || (stableValue == bottomPartStable && stableValue == leftPartStable)   || (stableValue == leftBottomPartStable  && stableValue != leftTopPartStable     && stableValue != leftPartStable   && stableValue != bottomPartStable && stableValue != rightBottomPartStable)) * stableValue;
			uint BRBStabilityCandidate = ((stableValue == bottomPartStable) || (stableValue == rightPartStable)  || (stableValue == rightBottomPartStable) || (stableValue == leftPartStable   && stableValue == topPartStable)    || (stableValue == leftTopPartStable     && stableValue != rightTopPartStable    && stableValue != topPartStable    && stableValue != leftPartStable   && stableValue != leftBottomPartStable) ) * stableValue;
			uint BLBStabilityCandidate = ((stableValue == leftPartStable)   || (stableValue == bottomPartStable) || (stableValue == leftBottomPartStable)  || (stableValue == topPartStable    && stableValue == rightPartStable)  || (stableValue == rightTopPartStable    && stableValue != rightBottomPartStable && stableValue != rightPartStable  && stableValue != topPartStable    && stableValue != leftTopPartStable)    ) * stableValue;

			uint BTStabilityCandidate = ((stableValue == topPartStable)    || (stableValue == leftTopPartStable     && stableValue != leftPartStable)   || (stableValue == rightTopPartStable    && stableValue != rightPartStable)  || (stableValue == leftBottomPartStable  && stableValue == rightBottomPartStable && stableValue != leftPartStable   && stableValue != rightPartStable)  || (stableValue == bottomPartStable && stableValue != leftPartStable   && stableValue != rightPartStable)  || loneStable) * stableValue;
			uint BRStabilityCandidate = ((stableValue == rightPartStable)  || (stableValue == rightTopPartStable    && stableValue != topPartStable)    || (stableValue == rightBottomPartStable && stableValue != bottomPartStable) || (stableValue == leftTopPartStable     && stableValue == leftBottomPartStable  && stableValue != topPartStable    && stableValue != bottomPartStable) || (stableValue == leftPartStable   && stableValue != topPartStable    && stableValue != bottomPartStable) || loneStable) * stableValue;
			uint BBStabilityCandidate = ((stableValue == bottomPartStable) || (stableValue == rightBottomPartStable && stableValue != rightPartStable)  || (stableValue == leftBottomPartStable  && stableValue != leftPartStable)   || (stableValue == rightTopPartStable    && stableValue == leftTopPartStable     && stableValue != rightPartStable  && stableValue != leftPartStable)   || (stableValue == topPartStable    && stableValue != rightPartStable  && stableValue != leftPartStable)   || loneStable) * stableValue;
			uint BLStabilityCandidate = ((stableValue == leftPartStable)   || (stableValue == leftBottomPartStable  && stableValue != bottomPartStable) || (stableValue == leftTopPartStable     && stableValue != topPartStable)    || (stableValue == rightBottomPartStable && stableValue == rightTopPartStable    && stableValue != bottomPartStable && stableValue != topPartStable)    || (stableValue == rightPartStable  && stableValue != bottomPartStable && stableValue != topPartStable)    || loneStable) * stableValue;

			uint ETLStabilityCandidate = ((stableValue == topPartStable)    || (stableValue == leftTopPartStable     && stableValue != leftPartStable))   * stableValue;
			uint ETRStabilityCandidate = ((stableValue == topPartStable)    || (stableValue == rightTopPartStable    && stableValue != rightPartStable))  * stableValue;
			uint ERTStabilityCandidate = ((stableValue == rightPartStable)  || (stableValue == rightTopPartStable    && stableValue != topPartStable))    * stableValue;
			uint ERBStabilityCandidate = ((stableValue == rightPartStable)  || (stableValue == rightBottomPartStable && stableValue != bottomPartStable)) * stableValue;
			uint EBRStabilityCandidate = ((stableValue == bottomPartStable) || (stableValue == rightBottomPartStable && stableValue != rightPartStable))  * stableValue;
			uint EBLStabilityCandidate = ((stableValue == bottomPartStable) || (stableValue == leftBottomPartStable  && stableValue != leftPartStable))   * stableValue;
			uint ELBStabilityCandidate = ((stableValue == leftPartStable)   || (stableValue == leftBottomPartStable  && stableValue != bottomPartStable)) * stableValue;
			uint ELTStabilityCandidate = ((stableValue == leftPartStable)   || (stableValue == leftTopPartStable     && stableValue != topPartStable))    * stableValue;

			uint CLTStabilityCandidate = (stableValue == leftTopPartStable     && stableValue != topPartStable    && stableValue != leftPartStable)   * stableValue;
			uint CRTStabilityCandidate = (stableValue == rightTopPartStable    && stableValue != rightPartStable  && stableValue != topPartStable)    * stableValue;
			uint CRBStabilityCandidate = (stableValue == rightBottomPartStable && stableValue != bottomPartStable && stableValue != rightPartStable)  * stableValue;
			uint CLBStabilityCandidate = (stableValue == leftBottomPartStable  && stableValue != leftPartStable   && stableValue != bottomPartStable) * stableValue;

			uint resBLTStable = max(BLTStabilityCandidate, LTCStabilityCandidate);
			uint resBRTStable = max(BRTStabilityCandidate, RTCStabilityCandidate);
			uint resBRBStable = max(BRBStabilityCandidate, RBCStabilityCandidate);
			uint resBLBStable = max(BLBStabilityCandidate, LBCStabilityCandidate);

			uint resETLStable = max(ETLStabilityCandidate, LTCStabilityCandidate);
			uint resETRStable = max(ETRStabilityCandidate, RTCStabilityCandidate);
			uint resERTStable = max(ERTStabilityCandidate, RTCStabilityCandidate);
			uint resERBStable = max(ERBStabilityCandidate, RBCStabilityCandidate);
			uint resEBRStable = max(EBRStabilityCandidate, RBCStabilityCandidate);
			uint resEBLStable = max(EBLStabilityCandidate, LBCStabilityCandidate);
			uint resELBStable = max(ELBStabilityCandidate, LBCStabilityCandidate);
			uint resELTStable = max(ELTStabilityCandidate, LTCStabilityCandidate);

			uint resCLTStable = max(CLTStabilityCandidate, LTCStabilityCandidate);
			uint resCRTStable = max(CRTStabilityCandidate, RTCStabilityCandidate);
			uint resCRBStable = max(CRBStabilityCandidate, RBCStabilityCandidate);
			uint resCLBStable = max(CLBStabilityCandidate, LBCStabilityCandidate);

			float cellStabilityPower = (float)stableValue          / (gDomainSize - 1.0f);
			float BLTStabilityPower  = (float)resBLTStable         / (gDomainSize - 1.0f);
			float BRTStabilityPower  = (float)resBRTStable         / (gDomainSize - 1.0f);
			float BRBStabilityPower  = (float)resBRBStable         / (gDomainSize - 1.0f);
			float BLBStabilityPower  = (float)resBLBStable         / (gDomainSize - 1.0f);
			float BTStabilityPower   = (float)BTStabilityCandidate / (gDomainSize - 1.0f);
			float BRStabilityPower   = (float)BRStabilityCandidate / (gDomainSize - 1.0f);
			float BBStabilityPower   = (float)BBStabilityCandidate / (gDomainSize - 1.0f);
			float BLStabilityPower   = (float)BLStabilityCandidate / (gDomainSize - 1.0f);
			float ETLStabilityPower  = (float)resETLStable         / (gDomainSize - 1.0f);
			float ETRStabilityPower  = (float)resETRStable         / (gDomainSize - 1.0f);
			float ERTStabilityPower  = (float)resERTStable         / (gDomainSize - 1.0f);
			float ERBStabilityPower  = (float)resERBStable         / (gDomainSize - 1.0f);
			float EBRStabilityPower  = (float)resEBRStable         / (gDomainSize - 1.0f);
			float EBLStabilityPower  = (float)resEBLStable         / (gDomainSize - 1.0f);
			float ELBStabilityPower  = (float)resELBStable         / (gDomainSize - 1.0f);
			float ELTStabilityPower  = (float)resELTStable         / (gDomainSize - 1.0f);
			float CLTStabilityPower  = (float)resCLTStable         / (gDomainSize - 1.0f);
			float CRTStabilityPower  = (float)resCRTStable         / (gDomainSize - 1.0f);
			float CRBStabilityPower  = (float)resCRBStable         / (gDomainSize - 1.0f);
			float CLBStabilityPower  = (float)resCLBStable         / (gDomainSize - 1.0f);

			float stabilityPower = cellStabilityPower * insideCentralOctagon + BLTStabilityPower * insideTopLeftBeamCorner + BRTStabilityPower * insideTopRightBeamCorner + BRBStabilityPower * insideBottomRightBeamCorner + BLBStabilityPower * insideBottomLeftBeamCorner + BTStabilityPower * insideTopBeamEnd + BRStabilityPower * insideRightBeamEnd + BBStabilityPower * insideBottomBeamEnd + BLStabilityPower * insideLeftBeamEnd + ETLStabilityPower * insideTopLeftEdge + ETRStabilityPower * insideTopRightEdge + ERTStabilityPower * insideRightTopEdge + ERBStabilityPower * insideRightBottomEdge + EBRStabilityPower * insideBottomRightEdge + EBLStabilityPower * insideBottomLeftEdge + ELBStabilityPower * insideLeftBottomEdge + ELTStabilityPower * insideLeftTopEdge + CLTStabilityPower * insideTopLeftCorner + CRTStabilityPower * insideTopRightCorner + CRBStabilityPower * insideBottomRightCorner + CLBStabilityPower * insideBottomLeftCorner;
			result               = lerp(result, colorStable, stabilityPower);
		}
	}
	else //Edge of the cell
	{
		result = gColorBetween;
	}

	Result[DTid.xy] = result;
}