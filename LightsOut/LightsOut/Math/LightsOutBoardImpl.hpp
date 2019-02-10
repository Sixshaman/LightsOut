#pragma once

#include <boost\dynamic_bitset.hpp>
#include <vector>

class LightsOutBoardImpl
{
public:
	LightsOutBoardImpl(uint16_t gameSize);
	virtual ~LightsOutBoardImpl();

	uint32_t CellCount() const;
	uint16_t Size()      const;

	virtual uint16_t DomainSize() const = 0;

	virtual void Reset(uint16_t gameSize) = 0;

	virtual void     Set(uint16_t x, uint16_t y, uint16_t value)       = 0;
	virtual uint16_t Get(uint16_t x, uint16_t y)                 const = 0;

	virtual void IncrementCell(uint16_t x, uint16_t y) = 0;

	virtual void Flip()          = 0;
	virtual void RotateNonZero() = 0;

	virtual bool IsNone() const = 0;

	virtual void CopyMemoryData(uint32_t* buf, uint32_t maxSize) const = 0;

protected:
	uint16_t mSize;
};

class LightsOutBinaryBoardImpl: public LightsOutBoardImpl
{
public:
	LightsOutBinaryBoardImpl(uint16_t gameSize);
	~LightsOutBinaryBoardImpl();

	uint16_t DomainSize() const override;

	void Reset(uint16_t gameSize) override;

	void     Set(uint16_t x, uint16_t y, uint16_t value)       override;
	uint16_t Get(uint16_t x, uint16_t y)                 const override;

	void IncrementCell(uint16_t x, uint16_t y) override;

	void AddBoard(const LightsOutBinaryBoardImpl* right);
	void MulBoard(const LightsOutBinaryBoardImpl* right);

	void IncDifBoard(const LightsOutBinaryBoardImpl* right1, const LightsOutBinaryBoardImpl* right2);

	bool IsEqual(const LightsOutBinaryBoardImpl* right);

	void Flip()          override;
	void RotateNonZero() override;

	bool IsNone() const override;

	void CopyMemoryData(uint32_t* buf, uint32_t maxSize) const override;

private:
	boost::dynamic_bitset<uint32_t> mBoard;
};

class LightsOutNaryBoardImpl: public LightsOutBoardImpl
{
public:
	LightsOutNaryBoardImpl(uint16_t gameSize, uint16_t domainSize);
	~LightsOutNaryBoardImpl();

	uint16_t DomainSize() const override;

	void Reset(uint16_t gameSize) override;

	void     Set(uint16_t x, uint16_t y, uint16_t value)       override;
	uint16_t Get(uint16_t x, uint16_t y)                 const override;

	void IncrementCell(uint16_t x, uint16_t y) override;

	void AddBoard(const LightsOutNaryBoardImpl* right);
	void SubBoard(const LightsOutNaryBoardImpl* right);
	void MulBoard(const LightsOutNaryBoardImpl* right);

	void MulBoardNum(uint16_t mul);
	void SubMulBoard(const LightsOutNaryBoardImpl* right, uint16_t mul);
	void IncDifBoard(const LightsOutNaryBoardImpl* right1, const LightsOutNaryBoardImpl* right2);

	bool IsEqual(const LightsOutNaryBoardImpl* right);

	void Flip()          override;
	void RotateNonZero() override;
	void InvertValues();

	bool IsNone() const override;

	void CopyMemoryData(uint32_t* buf, uint32_t maxSize) const override;

private:
	std::vector<uint32_t> mBoard;
	uint16_t mDomainSize;
};