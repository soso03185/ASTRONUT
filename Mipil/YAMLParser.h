#pragma once

struct BlockPOD;
struct PartPOD;
enum class eBlockType;
enum class ePartType;

// �ۿ����� ���� �о����
// ��� ���ʷ����� ���۽�Ŵ
// ��� ������ �׸���Ŵ����� ������ �� �־�� �Ѵ�.
// �׷��Ƿ� ��� ���� ��ü�� ���忡 �����ϱ�
// 

// ������ �Ľ��ϴ� Ŭ����
// ��� ���ʷ����� �ϳ� �� �����
class YAMLParser
{
public:
	YAMLParser() = default;
	~YAMLParser() = default;

	void ReadBlockPOD(std::string_view filePath, std::vector<std::shared_ptr<BlockPOD>>& blockPODs);
	void ReadPartPOD(std::string_view filePath, std::vector<std::shared_ptr<PartPOD>>& partPODs);

private:
	eBlockType GetBlockType(std::string_view blockName);
	ePartType GetPartType(std::string_view partName);
};

