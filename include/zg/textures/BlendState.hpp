#pragma once
namespace zg::textures
{
	enum class BlendFactor
	{
		Zero = 0,
		One = 1,
    	SrcColor = 2,
    	OneMinusSrcColor = 3,
    	DstColor = 4,
    	OneMinusDstColor = 5,
    	SrcAlpha = 6,
    	OneMinusSrcAlpha = 7,
    	DstAlpha = 8,
    	OneMinusDstAlpha = 9,
    	ConstantColor = 10,
    	OneMinusConstantColor = 11,
    	ConstantAlpha = 12,
    	OneMinusConstantAlpha = 13,
    	SrcAlphaSaturate = 14,
    	Src1Color = 15,
    	OneMinusSrc1Color = 16,
    	Src1Alpha = 17,
    	OneMinusSrc1Alpha = 18
	};
	enum class BlendOp
	{
		Add = 0,
		Subtract = 1,
		ReverseSubtract = 2,
		Min = 3,
		Max = 4
	};
	struct BlendState
	{
		bool enable = true;
		BlendFactor srcColor = BlendFactor::SrcAlpha;
		BlendFactor dstColor = BlendFactor::OneMinusSrcColor;
		BlendFactor srcAlpha = BlendFactor::SrcAlpha;
		BlendFactor dstAlpha = BlendFactor::OneMinusSrcAlpha;
		BlendOp colorOp = BlendOp::Add;
        BlendOp alphaOp = BlendOp::Add;
        static BlendState MainFramebuffer;
        static BlendState Layout;
        static BlendState Text;
		static BlendState SrcAlpha;
	};
}