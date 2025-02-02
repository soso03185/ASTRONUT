#pragma once
class FontManager;
/// <summary>
/// TextRenderer
///	CommonApp::m_pInstance->GetRenderer()->DrawTextRequest(L"����", x��ǥ(dx��ǥ�����), y��ǥ(dx��ǥ�����), ��(D2D1::ColorF), L"��Ʈ�̸�", ��Ʈ������, ��Ʈ�β�(���� ����, ����Ʈ 400), ��Ʈ�����(���� ����, ����Ʈ �븻), ��Ʈ���̱�(���� ����, ����Ʈ 5));
/// �̷��� ���� ��.
/// </summary>

class TextRenderer
{
public:
	TextRenderer();
	~TextRenderer();

	void DrawTextRequest(std::wstring_view text,
		FLOAT x,
		FLOAT y,
		D2D1::ColorF color,
		std::wstring_view fontFamily,
		FLOAT fontSize,
		float clientWidth,
		float clientHeight,
		DWRITE_FONT_WEIGHT fontWeight = DWRITE_FONT_WEIGHT_NORMAL,
		DWRITE_FONT_STYLE fontStyle = DWRITE_FONT_STYLE_NORMAL,
		DWRITE_FONT_STRETCH fontStretch = DWRITE_FONT_STRETCH_NORMAL);

	void DrawTextInQueue();

	void Initailize(ID2D1RenderTarget* pd2dRenderTarget, IDWriteFactory* pdWriteFactory);

private:
	struct TextLayoutKey
	{
		std::wstring text;
		std::wstring fontFamily;
		FLOAT fontSize;
		DWRITE_FONT_WEIGHT fontWeight;
		DWRITE_FONT_STYLE fontStyle;
		DWRITE_FONT_STRETCH fontStretch;

		std::wstring ToString() const
		{
			return text + fontFamily + std::to_wstring(fontSize)
				+ std::to_wstring(static_cast<int>(fontWeight))
				+ std::to_wstring(static_cast<int>(fontStyle))
				+ std::to_wstring(static_cast<int>(fontStretch));
		}
	};

	struct TextLayoutInfo
	{
		Microsoft::WRL::ComPtr<IDWriteTextFormat> textFormat;
		Microsoft::WRL::ComPtr<IDWriteTextLayout> textLayout;
	};

	struct TextRenderRequest
	{
		TextLayoutKey textLayoutKey;
		D2D1::ColorF color;
		Math::Vector2 pos;

		TextRenderRequest(const TextLayoutKey& key, const D2D1::ColorF& c, const Math::Vector2& pos)
			: textLayoutKey(key), color(c), pos(pos) {}
	};

	void RenderString(std::wstring_view text,
		FLOAT x,
		FLOAT y,
		D2D1::ColorF color,
		std::wstring_view fontFamily,
		FLOAT fontSize,
		DWRITE_FONT_WEIGHT fontWeight = DWRITE_FONT_WEIGHT_NORMAL,
		DWRITE_FONT_STYLE fontStyle = DWRITE_FONT_STYLE_NORMAL,
		DWRITE_FONT_STRETCH fontStretch = DWRITE_FONT_STRETCH_NORMAL);

	ID2D1RenderTarget* m_pd2dRenderTarget;
	IDWriteFactory* m_pdWriteFactory;
	std::unordered_map<std::wstring, TextLayoutInfo> m_textLayoutMap;
	std::queue<TextRenderRequest> m_renderQueue;
	std::unique_ptr<FontManager> m_fontManager;
};