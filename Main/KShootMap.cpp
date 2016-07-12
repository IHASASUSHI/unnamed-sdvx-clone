#include "stdafx.h"
#include "KShootMap.hpp"
#include "Profiling.hpp"

String KShootTick::ToString() const
{
	return Sprintf("%s|%s|%s", *buttons, *fx, *laser);
}
void KShootTick::Clear()
{
	buttons = "0000";
	fx = "00";
	laser = "--";
}

KShootTime::KShootTime() : block(-1), tick(-1)
{
}
KShootTime::KShootTime(uint32_t block, uint32_t tick) : block(block), tick(tick)
{
}
KShootTime::operator bool() const
{ 
	return block != -1; 
}

KShootMap::TickIterator::TickIterator(KShootMap& map, KShootTime start /*= KShootTime(0, 0)*/) : m_time(start), m_map(map)
{
	if(!m_map.GetBlock(m_time, m_currentBlock))
		m_currentBlock = nullptr;
}
KShootMap::TickIterator& KShootMap::TickIterator::operator++()
{
	m_time.tick++;
	if(m_time.tick >= m_currentBlock->ticks.size())
	{
		m_time.tick = 0;
		m_time.block++;
		if(!m_map.GetBlock(m_time, m_currentBlock))
			m_currentBlock = nullptr;
	}
	return *this;
}
KShootMap::TickIterator::operator bool() const
{
	return m_currentBlock != nullptr;
}
KShootTick& KShootMap::TickIterator::operator*()
{
	return m_currentBlock->ticks[m_time.tick];
}
KShootTick* KShootMap::TickIterator::operator->()
{
	return &m_currentBlock->ticks[m_time.tick];
}
const KShootTime& KShootMap::TickIterator::GetTime() const
{
	return m_time;
}
const KShootBlock& KShootMap::TickIterator::GetCurrentBlock() const
{
	return *m_currentBlock;
}

KShootMap::KShootMap()
{

}
KShootMap::~KShootMap()
{

}
bool KShootMap::Init(BinaryStream& input, bool metadataOnly)
{
	ProfilerScope $("Load KShootMap");

	// Read Byte Order Mark
	uint32_t bom = 0;
	input.Serialize(&bom, 3);
	if(bom != 0x00bfbbef) // Expected format for UTF-8
	{
		input.Seek(0);
	}

	uint32_t lineNumber = 0;
	String line;
	static const String lineEnding = "\r\n";

	// Parse Header
	while(TextStream::ReadLine(input, line, lineEnding))
	{
		line.Trim();
		lineNumber++;
		if(line == c_sep)
		{
			break;
		}
		String k, v;
		if(line.empty())
			continue;
		if(!line.Split("=", &k, &v))
			return false;
		WString str = Utility::ConvertToWString(v);
		settings.FindOrAdd(k) = v;
	}

	if(metadataOnly)
		return true;

	// Line by line parser
	KShootBlock block;
	KShootTick tick;
	KShootTime time = KShootTime(0, 0);
	while(TextStream::ReadLine(input, line, lineEnding))
	{
		if(line.empty())
		{
			break;
		}

		lineNumber++;
		if(line == c_sep)
		{
			// End this block
			blocks.push_back(block);
			block = KShootBlock(); // Reset block
			time.block++;
			time.tick = 0;
		}
		else
		{
			String k, v;
			if(line.Split("=", &k, &v))
			{
				tick.settings.FindOrAdd(k) = v;
			}
			else
			{
				// Parse tick content string 
				// The format looks like:
				// buttons*4|fx buttons*2|lasers*2 + additional things?
				// (fx) buttons are either '1' for normal '2' for hold, '0' for nothing
				//
				// lasers use a char to indicate position from left to right ASCII characters '0' -> 'o' respectively
				// '-' means no laser, ':' indicates a linear interpolation from previous point to the last point

				line.Split("|", &tick.buttons, &tick.fx);
				tick.fx.Split("|", &tick.fx, &tick.laser);
				if(tick.buttons.length() != 4)
				{
					Logf("Invalid buttons at line %d", Logger::Error, lineNumber);
					return false;
				}
				if(tick.fx.length() != 2)
				{
					Logf("Invalid FX buttons at line %d", Logger::Error, lineNumber);
					return false;
				}
				if(tick.laser.length() < 2)
				{
					Logf("Invalid lasers at line %d", Logger::Error, lineNumber);
					return false;
				}
				if(tick.laser.length() > 2)
				{
					tick.add = tick.laser.substr(2);
					tick.laser = tick.laser.substr(0, 2);
				}

				block.ticks.push_back(tick);
				tick = KShootTick(); // Reset tick
				time.tick++;
			}
		}
	}

	return true;
}
bool KShootMap::GetBlock(const KShootTime& time, KShootBlock*& tickOut)
{
	if(!time)
		return false;
	if(time.block >= blocks.size())
		return false;
	tickOut = &blocks[time.block];
	return true;
}
bool KShootMap::GetTick(const KShootTime& time, KShootTick*& tickOut)
{
	if(!time)
		return false;
	if(time.block >= blocks.size())
		return false;
	KShootBlock& b = blocks[time.block];
	if(time.tick >= b.ticks.size() || time.tick < 0)
		return false;
	tickOut = &b.ticks[time.tick];
	return true;
}
float KShootMap::TimeToFloat(const KShootTime& time) const
{
	KShootBlock* block;
	if(!const_cast<KShootMap*>(this)->GetBlock(time, block))
		return -1.0f;
	float seg = (float)time.tick / (float)block->ticks.size();
	return (float)time.block + seg;
}
float KShootMap::TranslateLaserChar(char c) const
{
	return (float)((uint32_t)c - c_laserStart) / (float)c_laserRange;
}
const char* KShootMap::c_sep = "--";
const uint32_t KShootMap::c_laserStart = '0';
const uint32_t KShootMap::c_laserEnd = 'o';
const uint32_t KShootMap::c_laserRange = KShootMap::c_laserEnd - KShootMap::c_laserStart;