#ifndef OLC_PGE_DEF
#define OLC_PGE_DEF

#pragma region std_includes
// O------------------------------------------------------------------------------O
// | STANDARD INCLUDES                                                            |
// O------------------------------------------------------------------------------O
#include <cmath>
#include <cstdint>
#include <string>
#include <iostream>
#include <streambuf>
#include <sstream>
#include <chrono>
#include <vector>
#include <list>
#include <thread>
#include <atomic>
#include <fstream>
#include <map>
#include <functional>
#include <algorithm>
#include <array>
#include <cstring>
#pragma endregion

#define PGE_VER 223

// O------------------------------------------------------------------------------O
// | COMPILER CONFIGURATION ODDITIES                                              |
// O------------------------------------------------------------------------------O
#pragma region compiler_config
#define USE_EXPERIMENTAL_FS
#if defined(_WIN32)
#if _MSC_VER >= 1920 && _MSVC_LANG >= 201703L
		#undef USE_EXPERIMENTAL_FS
	#endif
#endif
#if defined(__linux__) || defined(__MINGW32__) || defined(__EMSCRIPTEN__) || defined(__FreeBSD__) || defined(__APPLE__)
#if __cplusplus >= 201703L
#undef USE_EXPERIMENTAL_FS
#endif
#endif

#if !defined(OLC_KEYBOARD_UK)
#define OLC_KEYBOARD_UK
#endif


#if defined(USE_EXPERIMENTAL_FS) || defined(FORCE_EXPERIMENTAL_FS)
// C++14
	#define _SILENCE_EXPERIMENTAL_FILESYSTEM_DEPRECATION_WARNING
	#include <experimental/filesystem>
	namespace _gfs = std::experimental::filesystem::v1;
#else
// C++17
#include <filesystem>
namespace _gfs = std::filesystem;
#endif

#if defined(UNICODE) || defined(_UNICODE)
#define olcT(s) L##s
#else
#define olcT(s) s
#endif

#define UNUSED(x) (void)(x)

// O------------------------------------------------------------------------------O
// | PLATFORM SELECTION CODE, Thanks slavka!                                      |
// O------------------------------------------------------------------------------O

#if defined(OLC_PGE_HEADLESS)
#define OLC_PLATFORM_HEADLESS
	#define OLC_GFX_HEADLESS
	#if !defined(OLC_IMAGE_STB) && !defined(OLC_IMAGE_GDI) && !defined(OLC_IMAGE_LIBPNG)
		#define OLC_IMAGE_HEADLESS
	#endif
#endif

// Platform
#if !defined(OLC_PLATFORM_WINAPI) && !defined(OLC_PLATFORM_X11) && !defined(OLC_PLATFORM_GLUT) && !defined(OLC_PLATFORM_EMSCRIPTEN) && !defined(OLC_PLATFORM_HEADLESS)
#if !defined(OLC_PLATFORM_CUSTOM_EX)
#if defined(_WIN32)
#define OLC_PLATFORM_WINAPI
#endif
#if defined(__linux__) || defined(__FreeBSD__)
#define OLC_PLATFORM_X11
#endif
#if defined(__APPLE__)
#define GL_SILENCE_DEPRECATION
#define OLC_PLATFORM_GLUT
#endif
#if defined(__EMSCRIPTEN__)
#define OLC_PLATFORM_EMSCRIPTEN
#endif
#endif
#endif

// Start Situation
#if defined(OLC_PLATFORM_GLUT) || defined(OLC_PLATFORM_EMSCRIPTEN)
#define PGE_USE_CUSTOM_START
#endif



// Renderer
#if !defined(OLC_GFX_OPENGL10) && !defined(OLC_GFX_OPENGL33) && !defined(OLC_GFX_DIRECTX10) && !defined(OLC_GFX_HEADLESS)
#if !defined(OLC_GFX_CUSTOM_EX)
#if defined(OLC_PLATFORM_EMSCRIPTEN)
#define OLC_GFX_OPENGL33
#else
#define OLC_GFX_OPENGL10
#endif
#endif
#endif

// Image loader
#if !defined(OLC_IMAGE_STB) && !defined(OLC_IMAGE_GDI) && !defined(OLC_IMAGE_LIBPNG) && !defined(OLC_IMAGE_HEADLESS)
#if !defined(OLC_IMAGE_CUSTOM_EX)
#if defined(_WIN32)
#define	OLC_IMAGE_GDI
#endif
#if defined(__linux__) || defined(__APPLE__) || defined(__FreeBSD__) || defined(__EMSCRIPTEN__)
#define	OLC_IMAGE_LIBPNG
#endif
#endif
#endif


// O------------------------------------------------------------------------------O
// | PLATFORM-SPECIFIC DEPENDENCIES                                               |
// O------------------------------------------------------------------------------O
#if !defined(OLC_PGE_HEADLESS)
#if defined(OLC_PLATFORM_WINAPI)
#define _WINSOCKAPI_ // Thanks Cornchipss
		#if !defined(VC_EXTRALEAN)
		#define VC_EXTRALEAN
	#endif
	#if !defined(NOMINMAX)
		#define NOMINMAX
	#endif

	// In Code::Blocks
	#if !defined(_WIN32_WINNT)
		#ifdef HAVE_MSMF
			#define _WIN32_WINNT 0x0600 // Windows Vista
		#else
			#define _WIN32_WINNT 0x0500 // Windows 2000
		#endif
	#endif

	#include <windows.h>
	#undef _WINSOCKAPI_
#endif

#if defined(OLC_PLATFORM_X11)
namespace X11
	{
		#include <X11/X.h>
		#include <X11/Xlib.h>
	}
#endif

#if defined(OLC_PLATFORM_GLUT)
#if defined(__linux__)
#include <GL/glut.h>
		#include <GL/freeglut_ext.h>
#endif
#if defined(__APPLE__)
#include <GLUT/glut.h>
#include <objc/message.h>
#include <objc/NSObjCRuntime.h>
#endif
#endif
#endif

#if defined(OLC_PGE_HEADLESS)
#if defined max
#undef max
#endif
#if defined min
#undef min
#endif
#endif
#pragma endregion

// O------------------------------------------------------------------------------O
// | olcPixelGameEngine INTERFACE DECLARATION                                     |
// O------------------------------------------------------------------------------O
#pragma region pge_declaration
namespace olc
{
    class PixelGameEngine;
    class Sprite;

    // Pixel Game Engine Advanced Configuration
    constexpr uint8_t  nMouseButtons = 5;
    constexpr uint8_t  nDefaultAlpha = 0xFF;
    constexpr uint32_t nDefaultPixel = (nDefaultAlpha << 24);
    constexpr uint8_t  nTabSizeInSpaces = 4;
    constexpr size_t OLC_MAX_VERTS = 128;
    enum rcode { FAIL = 0, OK = 1, NO_FILE = -1 };

    // O------------------------------------------------------------------------------O
    // | olc::Pixel - Represents a 32-Bit RGBA colour                                 |
    // O------------------------------------------------------------------------------O
    struct Pixel
    {
        union
        {
            uint32_t n = nDefaultPixel;
            struct { uint8_t r; uint8_t g; uint8_t b; uint8_t a; };
        };

        enum Mode { NORMAL, MASK, ALPHA, CUSTOM };

        Pixel();
        Pixel(uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha = nDefaultAlpha);
        Pixel(uint32_t p);
        Pixel& operator = (const Pixel& v) = default;
        bool   operator ==(const Pixel& p) const;
        bool   operator !=(const Pixel& p) const;
        Pixel  operator * (const float i) const;
        Pixel  operator / (const float i) const;
        Pixel& operator *=(const float i);
        Pixel& operator /=(const float i);
        Pixel  operator + (const Pixel& p) const;
        Pixel  operator - (const Pixel& p) const;
        Pixel& operator +=(const Pixel& p);
        Pixel& operator -=(const Pixel& p);
        Pixel  operator * (const Pixel& p) const;
        Pixel& operator *=(const Pixel& p);
        Pixel  inv() const;
    };

    Pixel PixelF(float red, float green, float blue, float alpha = 1.0f);
    Pixel PixelLerp(const olc::Pixel& p1, const olc::Pixel& p2, float t);


    // O------------------------------------------------------------------------------O
    // | USEFUL CONSTANTS                                                             |
    // O------------------------------------------------------------------------------O
    static const Pixel
            GREY(192, 192, 192), DARK_GREY(128, 128, 128), VERY_DARK_GREY(64, 64, 64),
            RED(255, 0, 0), DARK_RED(128, 0, 0), VERY_DARK_RED(64, 0, 0),
            YELLOW(255, 255, 0), DARK_YELLOW(128, 128, 0), VERY_DARK_YELLOW(64, 64, 0),
            GREEN(0, 255, 0), DARK_GREEN(0, 128, 0), VERY_DARK_GREEN(0, 64, 0),
            CYAN(0, 255, 255), DARK_CYAN(0, 128, 128), VERY_DARK_CYAN(0, 64, 64),
            BLUE(0, 0, 255), DARK_BLUE(0, 0, 128), VERY_DARK_BLUE(0, 0, 64),
            MAGENTA(255, 0, 255), DARK_MAGENTA(128, 0, 128), VERY_DARK_MAGENTA(64, 0, 64),
            WHITE(255, 255, 255), BLACK(0, 0, 0), BLANK(0, 0, 0, 0);

    // Thanks to scripticuk and others for updating the key maps
    // NOTE: The GLUT platform will need updating, open to contributions ;)
    enum Key
    {
        NONE,
        A, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P, Q, R, S, T, U, V, W, X, Y, Z,
        K0, K1, K2, K3, K4, K5, K6, K7, K8, K9,
        F1, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12,
        UP, DOWN, LEFT, RIGHT,
        SPACE, TAB, SHIFT, CTRL, INS, DEL, HOME, END, PGUP, PGDN,
        BACK, ESCAPE, RETURN, ENTER, PAUSE, SCROLL,
        NP0, NP1, NP2, NP3, NP4, NP5, NP6, NP7, NP8, NP9,
        NP_MUL, NP_DIV, NP_ADD, NP_SUB, NP_DECIMAL, PERIOD,
        EQUALS, COMMA, MINUS,
        OEM_1, OEM_2, OEM_3, OEM_4, OEM_5, OEM_6, OEM_7, OEM_8,
        CAPS_LOCK, ENUM_END
    };

    namespace Mouse
    {
        static constexpr int32_t LEFT = 0;
        static constexpr int32_t RIGHT = 1;
        static constexpr int32_t MIDDLE = 2;
    };

    // O------------------------------------------------------------------------------O
    // | olc::HWButton - Represents the state of a hardware button (mouse/key/joy)    |
    // O------------------------------------------------------------------------------O
    struct HWButton
    {
        bool bPressed = false;	// Set once during the frame the event occurs
        bool bReleased = false;	// Set once during the frame the event occurs
        bool bHeld = false;		// Set true for all frames between pressed and released events
    };




    // O------------------------------------------------------------------------------O
    // | olc::vX2d - A generic 2D vector type                                         |
    // O------------------------------------------------------------------------------O
#if !defined(OLC_IGNORE_VEC2D)
    template <class T>
    struct v2d_generic
    {
        T x = 0;
        T y = 0;
        v2d_generic() : x(0), y(0) {}
        v2d_generic(T _x, T _y) : x(_x), y(_y) {}
        v2d_generic(const v2d_generic& v) : x(v.x), y(v.y) {}
        v2d_generic& operator=(const v2d_generic& v) = default;
        T mag() const { return T(std::sqrt(x * x + y * y)); }
        T mag2() const { return x * x + y * y; }
        v2d_generic  norm() const { T r = 1 / mag(); return v2d_generic(x * r, y * r); }
        v2d_generic  perp() const { return v2d_generic(-y, x); }
        v2d_generic  floor() const { return v2d_generic(std::floor(x), std::floor(y)); }
        v2d_generic  ceil() const { return v2d_generic(std::ceil(x), std::ceil(y)); }
        v2d_generic  max(const v2d_generic& v) const { return v2d_generic(std::max(x, v.x), std::max(y, v.y)); }
        v2d_generic  min(const v2d_generic& v) const { return v2d_generic(std::min(x, v.x), std::min(y, v.y)); }
        v2d_generic  cart() { return { std::cos(y) * x, std::sin(y) * x }; }
        v2d_generic  polar() { return { mag(), std::atan2(y, x) }; }
        v2d_generic  clamp(const v2d_generic& v1, const v2d_generic& v2) const { return this->max(v1).min(v2); }
        v2d_generic	 lerp(const v2d_generic& v1, const double t) { return this->operator*(T(1.0 - t)) + (v1 * T(t)); }
        T dot(const v2d_generic& rhs) const { return this->x * rhs.x + this->y * rhs.y; }
        T cross(const v2d_generic& rhs) const { return this->x * rhs.y - this->y * rhs.x; }
        v2d_generic  operator +  (const v2d_generic& rhs) const { return v2d_generic(this->x + rhs.x, this->y + rhs.y); }
        v2d_generic  operator -  (const v2d_generic& rhs) const { return v2d_generic(this->x - rhs.x, this->y - rhs.y); }
        v2d_generic  operator *  (const T& rhs)           const { return v2d_generic(this->x * rhs, this->y * rhs); }
        v2d_generic  operator *  (const v2d_generic& rhs) const { return v2d_generic(this->x * rhs.x, this->y * rhs.y); }
        v2d_generic  operator /  (const T& rhs)           const { return v2d_generic(this->x / rhs, this->y / rhs); }
        v2d_generic  operator /  (const v2d_generic& rhs) const { return v2d_generic(this->x / rhs.x, this->y / rhs.y); }
        v2d_generic& operator += (const v2d_generic& rhs) { this->x += rhs.x; this->y += rhs.y; return *this; }
        v2d_generic& operator -= (const v2d_generic& rhs) { this->x -= rhs.x; this->y -= rhs.y; return *this; }
        v2d_generic& operator *= (const T& rhs) { this->x *= rhs; this->y *= rhs; return *this; }
        v2d_generic& operator /= (const T& rhs) { this->x /= rhs; this->y /= rhs; return *this; }
        v2d_generic& operator *= (const v2d_generic& rhs) { this->x *= rhs.x; this->y *= rhs.y; return *this; }
        v2d_generic& operator /= (const v2d_generic& rhs) { this->x /= rhs.x; this->y /= rhs.y; return *this; }
        v2d_generic  operator +  () const { return { +x, +y }; }
        v2d_generic  operator -  () const { return { -x, -y }; }
        bool operator == (const v2d_generic& rhs) const { return (this->x == rhs.x && this->y == rhs.y); }
        bool operator != (const v2d_generic& rhs) const { return (this->x != rhs.x || this->y != rhs.y); }
        const std::string str() const { return std::string("(") + std::to_string(this->x) + "," + std::to_string(this->y) + ")"; }
        friend std::ostream& operator << (std::ostream& os, const v2d_generic& rhs) { os << rhs.str(); return os; }
        operator v2d_generic<int32_t>() const { return { static_cast<int32_t>(this->x), static_cast<int32_t>(this->y) }; }
        operator v2d_generic<float>() const { return { static_cast<float>(this->x), static_cast<float>(this->y) }; }
        operator v2d_generic<double>() const { return { static_cast<double>(this->x), static_cast<double>(this->y) }; }
    };

    // Note: joshinils has some good suggestions here, but they are complicated to implement at this moment,
    // however they will appear in a future version of PGE
    template<class T> inline v2d_generic<T> operator * (const float& lhs, const v2d_generic<T>& rhs)
    { return v2d_generic<T>((T)(lhs * (float)rhs.x), (T)(lhs * (float)rhs.y)); }
    template<class T> inline v2d_generic<T> operator * (const double& lhs, const v2d_generic<T>& rhs)
    { return v2d_generic<T>((T)(lhs * (double)rhs.x), (T)(lhs * (double)rhs.y)); }
    template<class T> inline v2d_generic<T> operator * (const int& lhs, const v2d_generic<T>& rhs)
    { return v2d_generic<T>((T)(lhs * (int)rhs.x), (T)(lhs * (int)rhs.y)); }
    template<class T> inline v2d_generic<T> operator / (const float& lhs, const v2d_generic<T>& rhs)
    { return v2d_generic<T>((T)(lhs / (float)rhs.x), (T)(lhs / (float)rhs.y)); }
    template<class T> inline v2d_generic<T> operator / (const double& lhs, const v2d_generic<T>& rhs)
    { return v2d_generic<T>((T)(lhs / (double)rhs.x), (T)(lhs / (double)rhs.y)); }
    template<class T> inline v2d_generic<T> operator / (const int& lhs, const v2d_generic<T>& rhs)
    { return v2d_generic<T>((T)(lhs / (int)rhs.x), (T)(lhs / (int)rhs.y)); }

    // To stop dandistine crying...
    template<class T, class U> inline bool operator < (const v2d_generic<T>& lhs, const v2d_generic<U>& rhs)
    { return lhs.y < rhs.y || (lhs.y == rhs.y && lhs.x < rhs.x); }
    template<class T, class U> inline bool operator > (const v2d_generic<T>& lhs, const v2d_generic<U>& rhs)
    { return lhs.y > rhs.y || (lhs.y == rhs.y && lhs.x > rhs.x); }

    typedef v2d_generic<int32_t> vi2d;
    typedef v2d_generic<uint32_t> vu2d;
    typedef v2d_generic<float> vf2d;
    typedef v2d_generic<double> vd2d;
#endif






    // O------------------------------------------------------------------------------O
    // | olc::ResourcePack - A virtual scrambled filesystem to pack your assets into  |
    // O------------------------------------------------------------------------------O
    struct ResourceBuffer : public std::streambuf
    {
        ResourceBuffer(std::ifstream& ifs, uint32_t offset, uint32_t size);
        std::vector<char> vMemory;
    };

    class ResourcePack : public std::streambuf
    {
    public:
        ResourcePack();
        ~ResourcePack();
        bool AddFile(const std::string& sFile);
        bool LoadPack(const std::string& sFile, const std::string& sKey);
        bool SavePack(const std::string& sFile, const std::string& sKey);
        ResourceBuffer GetFileBuffer(const std::string& sFile);
        bool Loaded();
    private:
        struct sResourceFile { uint32_t nSize; uint32_t nOffset; };
        std::map<std::string, sResourceFile> mapFiles;
        std::ifstream baseFile;
        std::vector<char> scramble(const std::vector<char>& data, const std::string& key);
        std::string makeposix(const std::string& path);
    };


    class ImageLoader
    {
    public:
        ImageLoader() = default;
        virtual ~ImageLoader() = default;
        virtual olc::rcode LoadImageResource(olc::Sprite* spr, const std::string& sImageFile, olc::ResourcePack* pack) = 0;
        virtual olc::rcode SaveImageResource(olc::Sprite* spr, const std::string& sImageFile) = 0;
    };


    // O------------------------------------------------------------------------------O
    // | olc::Sprite - An image represented by a 2D array of olc::Pixel               |
    // O------------------------------------------------------------------------------O
    class Sprite
    {
    public:
        Sprite();
        Sprite(const std::string& sImageFile, olc::ResourcePack* pack = nullptr);
        Sprite(int32_t w, int32_t h);
        Sprite(const olc::Sprite&) = delete;
        ~Sprite();

    public:
        olc::rcode LoadFromFile(const std::string& sImageFile, olc::ResourcePack* pack = nullptr);

    public:
        int32_t width = 0;
        int32_t height = 0;
        enum Mode { NORMAL, PERIODIC, CLAMP };
        enum Flip { NONE = 0, HORIZ = 1, VERT = 2 };

    public:
        void SetSampleMode(olc::Sprite::Mode mode = olc::Sprite::Mode::NORMAL);
        Pixel GetPixel(int32_t x, int32_t y) const;
        bool  SetPixel(int32_t x, int32_t y, Pixel p);
        Pixel GetPixel(const olc::vi2d& a) const;
        bool  SetPixel(const olc::vi2d& a, Pixel p);
        Pixel Sample(float x, float y) const;
        Pixel Sample(const olc::vf2d& uv) const;
        Pixel SampleBL(float u, float v) const;
        Pixel SampleBL(const olc::vf2d& uv) const;
        Pixel* GetData();
        olc::Sprite* Duplicate();
        olc::Sprite* Duplicate(const olc::vi2d& vPos, const olc::vi2d& vSize);
        olc::vi2d Size() const;
        std::vector<olc::Pixel> pColData;
        Mode modeSample = Mode::NORMAL;

        static std::unique_ptr<olc::ImageLoader> loader;
    };

    // O------------------------------------------------------------------------------O
    // | olc::Decal - A GPU resident storage of an olc::Sprite                        |
    // O------------------------------------------------------------------------------O
    class Decal
    {
    public:
        Decal(olc::Sprite* spr, bool filter = false, bool clamp = true);
        Decal(const uint32_t nExistingTextureResource, olc::Sprite* spr);
        virtual ~Decal();
        void Update();
        void UpdateSprite();

    public: // But dont touch
        int32_t id = -1;
        olc::Sprite* sprite = nullptr;
        olc::vf2d vUVScale = { 1.0f, 1.0f };
    };

    enum class DecalMode
    {
        NORMAL,
        ADDITIVE,
        MULTIPLICATIVE,
        STENCIL,
        ILLUMINATE,
        WIREFRAME,
        MODEL3D,
    };

    enum class DecalStructure
    {
        LINE,
        FAN,
        STRIP,
        LIST
    };

    // O------------------------------------------------------------------------------O
    // | olc::Renderable - Convenience class to keep a sprite and decal together      |
    // O------------------------------------------------------------------------------O
    class Renderable
    {
    public:
        Renderable() = default;
        Renderable(Renderable&& r) : pSprite(std::move(r.pSprite)), pDecal(std::move(r.pDecal)) {}
        Renderable(const Renderable&) = delete;
        olc::rcode Load(const std::string& sFile, ResourcePack* pack = nullptr, bool filter = false, bool clamp = true);
        void Create(uint32_t width, uint32_t height, bool filter = false, bool clamp = true);
        olc::Decal* Decal() const;
        olc::Sprite* Sprite() const;

    private:
        std::unique_ptr<olc::Sprite> pSprite = nullptr;
        std::unique_ptr<olc::Decal> pDecal = nullptr;
    };


    // O------------------------------------------------------------------------------O
    // | Auxilliary components internal to engine                                     |
    // O------------------------------------------------------------------------------O

    struct DecalInstance
    {
        olc::Decal* decal = nullptr;
        std::vector<olc::vf2d> pos;
        std::vector<olc::vf2d> uv;
        std::vector<float> w;
        std::vector<olc::Pixel> tint;
        olc::DecalMode mode = olc::DecalMode::NORMAL;
        olc::DecalStructure structure = olc::DecalStructure::FAN;
        uint32_t points = 0;
    };

    struct LayerDesc
    {
        olc::vf2d vOffset = { 0, 0 };
        olc::vf2d vScale = { 1, 1 };
        bool bShow = false;
        bool bUpdate = false;
        olc::Renderable pDrawTarget;
        uint32_t nResID = 0;
        std::vector<DecalInstance> vecDecalInstance;
        olc::Pixel tint = olc::WHITE;
        std::function<void()> funcHook = nullptr;
    };

    class Renderer
    {
    public:
        virtual ~Renderer() = default;
        virtual void       PrepareDevice() = 0;
        virtual olc::rcode CreateDevice(std::vector<void*> params, bool bFullScreen, bool bVSYNC) = 0;
        virtual olc::rcode DestroyDevice() = 0;
        virtual void       DisplayFrame() = 0;
        virtual void       PrepareDrawing() = 0;
        virtual void	   SetDecalMode(const olc::DecalMode& mode) = 0;
        virtual void       DrawLayerQuad(const olc::vf2d& offset, const olc::vf2d& scale, const olc::Pixel tint) = 0;
        virtual void       DrawDecal(const olc::DecalInstance& decal) = 0;
        virtual uint32_t   CreateTexture(const uint32_t width, const uint32_t height, const bool filtered = false, const bool clamp = true) = 0;
        virtual void       UpdateTexture(uint32_t id, olc::Sprite* spr) = 0;
        virtual void       ReadTexture(uint32_t id, olc::Sprite* spr) = 0;
        virtual uint32_t   DeleteTexture(const uint32_t id) = 0;
        virtual void       ApplyTexture(uint32_t id) = 0;
        virtual void       UpdateViewport(const olc::vi2d& pos, const olc::vi2d& size) = 0;
        virtual void       ClearBuffer(olc::Pixel p, bool bDepth) = 0;
        static olc::PixelGameEngine* ptrPGE;
    };

    class Platform
    {
    public:
        virtual ~Platform() = default;
        virtual olc::rcode ApplicationStartUp() = 0;
        virtual olc::rcode ApplicationCleanUp() = 0;
        virtual olc::rcode ThreadStartUp() = 0;
        virtual olc::rcode ThreadCleanUp() = 0;
        virtual olc::rcode CreateGraphics(bool bFullScreen, bool bEnableVSYNC, const olc::vi2d& vViewPos, const olc::vi2d& vViewSize) = 0;
        virtual olc::rcode CreateWindowPane(const olc::vi2d& vWindowPos, olc::vi2d& vWindowSize, bool bFullScreen) = 0;
        virtual olc::rcode SetWindowTitle(const std::string& s) = 0;
        virtual olc::rcode StartSystemEventLoop() = 0;
        virtual olc::rcode HandleSystemEvent() = 0;
        static olc::PixelGameEngine* ptrPGE;
    };

    class PGEX;

    // The Static Twins (plus one)
    static std::unique_ptr<Renderer> renderer;
    static std::unique_ptr<Platform> platform;
    static std::map<size_t, uint8_t> mapKeys;

    // O------------------------------------------------------------------------------O
    // | olc::PixelGameEngine - The main BASE class for your application              |
    // O------------------------------------------------------------------------------O
    class PixelGameEngine
    {
    public:
        PixelGameEngine();
        virtual ~PixelGameEngine();
    public:
        olc::rcode Construct(int32_t screen_w, int32_t screen_h, int32_t pixel_w, int32_t pixel_h,
                             bool full_screen = false, bool vsync = false, bool cohesion = false);
        olc::rcode Start();

    public: // User Override Interfaces
        // Called once on application startup, use to load your resources
        virtual bool OnUserCreate();
        // Called every frame, and provides you with a time per frame value
        virtual bool OnUserUpdate(float fElapsedTime);
        // Called once on application termination, so you can be one clean coder
        virtual bool OnUserDestroy();

        // Called when a text entry is confirmed with "enter" key
        virtual void OnTextEntryComplete(const std::string& sText);
        // Called when a console command is executed
        virtual bool OnConsoleCommand(const std::string& sCommand);


    public: // Hardware Interfaces
        // Returns true if window is currently in focus
        bool IsFocused() const;
        // Get the state of a specific keyboard button
        HWButton GetKey(Key k) const;
        // Get the state of a specific mouse button
        HWButton GetMouse(uint32_t b) const;
        // Get Mouse X coordinate in "pixel" space
        int32_t GetMouseX() const;
        // Get Mouse Y coordinate in "pixel" space
        int32_t GetMouseY() const;
        // Get Mouse Wheel Delta
        int32_t GetMouseWheel() const;
        // Get the mouse in window space
        const olc::vi2d& GetWindowMouse() const;
        // Gets the mouse as a vector to keep Tarriest happy
        const olc::vi2d& GetMousePos() const;

        static const std::map<size_t, uint8_t>& GetKeyMap() { return mapKeys; }

    public: // Utility
        // Returns the width of the screen in "pixels"
        int32_t ScreenWidth() const;
        // Returns the height of the screen in "pixels"
        int32_t ScreenHeight() const;
        // Returns the width of the currently selected drawing target in "pixels"
        int32_t GetDrawTargetWidth() const;
        // Returns the height of the currently selected drawing target in "pixels"
        int32_t GetDrawTargetHeight() const;
        // Returns the currently active draw target
        olc::Sprite* GetDrawTarget() const;
        // Resize the primary screen sprite
        void SetScreenSize(int w, int h);
        // Specify which Sprite should be the target of drawing functions, use nullptr
        // to specify the primary screen
        void SetDrawTarget(Sprite* target);
        // Gets the current Frames Per Second
        uint32_t GetFPS() const;
        // Gets last update of elapsed time
        float GetElapsedTime() const;
        // Gets Actual Window size
        const olc::vi2d& GetWindowSize() const;
        // Gets pixel scale
        const olc::vi2d& GetPixelSize() const;
        // Gets actual pixel scale
        const olc::vi2d& GetScreenPixelSize() const;
        // Gets "screen" size
        const olc::vi2d& GetScreenSize() const;
        // Gets any files dropped this frame
        const std::vector<std::string>& GetDroppedFiles() const;
        const olc::vi2d& GetDroppedFilesPoint() const;

    public: // CONFIGURATION ROUTINES
        // Layer targeting functions
        void SetDrawTarget(uint8_t layer, bool bDirty = true);
        void EnableLayer(uint8_t layer, bool b);
        void SetLayerOffset(uint8_t layer, const olc::vf2d& offset);
        void SetLayerOffset(uint8_t layer, float x, float y);
        void SetLayerScale(uint8_t layer, const olc::vf2d& scale);
        void SetLayerScale(uint8_t layer, float x, float y);
        void SetLayerTint(uint8_t layer, const olc::Pixel& tint);
        void SetLayerCustomRenderFunction(uint8_t layer, std::function<void()> f);

        std::vector<LayerDesc>& GetLayers();
        uint32_t CreateLayer();

        // Change the pixel mode for different optimisations
        // olc::Pixel::NORMAL = No transparency
        // olc::Pixel::MASK   = Transparent if alpha is < 255
        // olc::Pixel::ALPHA  = Full transparency
        void SetPixelMode(Pixel::Mode m);
        Pixel::Mode GetPixelMode();
        // Use a custom blend function
        void SetPixelMode(std::function<olc::Pixel(const int x, const int y, const olc::Pixel& pSource, const olc::Pixel& pDest)> pixelMode);
        // Change the blend factor from between 0.0f to 1.0f;
        void SetPixelBlend(float fBlend);



    public: // DRAWING ROUTINES
        // Draws a single Pixel
        virtual bool Draw(int32_t x, int32_t y, Pixel p = olc::WHITE);
        bool Draw(const olc::vi2d& pos, Pixel p = olc::WHITE);
        // Draws a line from (x1,y1) to (x2,y2)
        void DrawLine(int32_t x1, int32_t y1, int32_t x2, int32_t y2, Pixel p = olc::WHITE, uint32_t pattern = 0xFFFFFFFF);
        void DrawLine(const olc::vi2d& pos1, const olc::vi2d& pos2, Pixel p = olc::WHITE, uint32_t pattern = 0xFFFFFFFF);
        // Draws a circle located at (x,y) with radius
        void DrawCircle(int32_t x, int32_t y, int32_t radius, Pixel p = olc::WHITE, uint8_t mask = 0xFF);
        void DrawCircle(const olc::vi2d& pos, int32_t radius, Pixel p = olc::WHITE, uint8_t mask = 0xFF);
        // Fills a circle located at (x,y) with radius
        void FillCircle(int32_t x, int32_t y, int32_t radius, Pixel p = olc::WHITE);
        void FillCircle(const olc::vi2d& pos, int32_t radius, Pixel p = olc::WHITE);
        // Draws a rectangle at (x,y) to (x+w,y+h)
        void DrawRect(int32_t x, int32_t y, int32_t w, int32_t h, Pixel p = olc::WHITE);
        void DrawRect(const olc::vi2d& pos, const olc::vi2d& size, Pixel p = olc::WHITE);
        // Fills a rectangle at (x,y) to (x+w,y+h)
        void FillRect(int32_t x, int32_t y, int32_t w, int32_t h, Pixel p = olc::WHITE);
        void FillRect(const olc::vi2d& pos, const olc::vi2d& size, Pixel p = olc::WHITE);
        // Draws a triangle between points (x1,y1), (x2,y2) and (x3,y3)
        void DrawTriangle(int32_t x1, int32_t y1, int32_t x2, int32_t y2, int32_t x3, int32_t y3, Pixel p = olc::WHITE);
        void DrawTriangle(const olc::vi2d& pos1, const olc::vi2d& pos2, const olc::vi2d& pos3, Pixel p = olc::WHITE);
        // Flat fills a triangle between points (x1,y1), (x2,y2) and (x3,y3)
        void FillTriangle(int32_t x1, int32_t y1, int32_t x2, int32_t y2, int32_t x3, int32_t y3, Pixel p = olc::WHITE);
        void FillTriangle(const olc::vi2d& pos1, const olc::vi2d& pos2, const olc::vi2d& pos3, Pixel p = olc::WHITE);
        // Fill a textured and coloured triangle
        void FillTexturedTriangle(const std::vector<olc::vf2d>& vPoints, std::vector<olc::vf2d> vTex, std::vector<olc::Pixel> vColour, olc::Sprite* sprTex);
        void FillTexturedPolygon(const std::vector<olc::vf2d>& vPoints, const std::vector<olc::vf2d>& vTex, const std::vector<olc::Pixel>& vColour, olc::Sprite* sprTex, olc::DecalStructure structure = olc::DecalStructure::LIST);
        // Draws an entire sprite at location (x,y)
        void DrawSprite(int32_t x, int32_t y, Sprite* sprite, uint32_t scale = 1, uint8_t flip = olc::Sprite::NONE);
        void DrawSprite(const olc::vi2d& pos, Sprite* sprite, uint32_t scale = 1, uint8_t flip = olc::Sprite::NONE);
        // Draws an area of a sprite at location (x,y), where the
        // selected area is (ox,oy) to (ox+w,oy+h)
        void DrawPartialSprite(int32_t x, int32_t y, Sprite* sprite, int32_t ox, int32_t oy, int32_t w, int32_t h, uint32_t scale = 1, uint8_t flip = olc::Sprite::NONE);
        void DrawPartialSprite(const olc::vi2d& pos, Sprite* sprite, const olc::vi2d& sourcepos, const olc::vi2d& size, uint32_t scale = 1, uint8_t flip = olc::Sprite::NONE);
        // Draws a single line of text - traditional monospaced
        void DrawString(int32_t x, int32_t y, const std::string& sText, Pixel col = olc::WHITE, uint32_t scale = 1);
        void DrawString(const olc::vi2d& pos, const std::string& sText, Pixel col = olc::WHITE, uint32_t scale = 1);
        olc::vi2d GetTextSize(const std::string& s);
        // Draws a single line of text - non-monospaced
        void DrawStringProp(int32_t x, int32_t y, const std::string& sText, Pixel col = olc::WHITE, uint32_t scale = 1);
        void DrawStringProp(const olc::vi2d& pos, const std::string& sText, Pixel col = olc::WHITE, uint32_t scale = 1);
        olc::vi2d GetTextSizeProp(const std::string& s);

        // Decal Quad functions
        void SetDecalMode(const olc::DecalMode& mode);
        void SetDecalStructure(const olc::DecalStructure& structure);
        // Draws a whole decal, with optional scale and tinting
        void DrawDecal(const olc::vf2d& pos, olc::Decal* decal, const olc::vf2d& scale = { 1.0f,1.0f }, const olc::Pixel& tint = olc::WHITE);
        // Draws a region of a decal, with optional scale and tinting
        void DrawPartialDecal(const olc::vf2d& pos, olc::Decal* decal, const olc::vf2d& source_pos, const olc::vf2d& source_size, const olc::vf2d& scale = { 1.0f,1.0f }, const olc::Pixel& tint = olc::WHITE);
        void DrawPartialDecal(const olc::vf2d& pos, const olc::vf2d& size, olc::Decal* decal, const olc::vf2d& source_pos, const olc::vf2d& source_size, const olc::Pixel& tint = olc::WHITE);
        // Draws fully user controlled 4 vertices, pos(pixels), uv(pixels), colours
        void DrawExplicitDecal(olc::Decal* decal, const olc::vf2d* pos, const olc::vf2d* uv, const olc::Pixel* col, uint32_t elements = 4);
        // Draws a decal with 4 arbitrary points, warping the texture to look "correct"
        void DrawWarpedDecal(olc::Decal* decal, const olc::vf2d(&pos)[4], const olc::Pixel& tint = olc::WHITE);
        void DrawWarpedDecal(olc::Decal* decal, const olc::vf2d* pos, const olc::Pixel& tint = olc::WHITE);
        void DrawWarpedDecal(olc::Decal* decal, const std::array<olc::vf2d, 4>& pos, const olc::Pixel& tint = olc::WHITE);
        // As above, but you can specify a region of a decal source sprite
        void DrawPartialWarpedDecal(olc::Decal* decal, const olc::vf2d(&pos)[4], const olc::vf2d& source_pos, const olc::vf2d& source_size, const olc::Pixel& tint = olc::WHITE);
        void DrawPartialWarpedDecal(olc::Decal* decal, const olc::vf2d* pos, const olc::vf2d& source_pos, const olc::vf2d& source_size, const olc::Pixel& tint = olc::WHITE);
        void DrawPartialWarpedDecal(olc::Decal* decal, const std::array<olc::vf2d, 4>& pos, const olc::vf2d& source_pos, const olc::vf2d& source_size, const olc::Pixel& tint = olc::WHITE);
        // Draws a decal rotated to specified angle, wit point of rotation offset
        void DrawRotatedDecal(const olc::vf2d& pos, olc::Decal* decal, const float fAngle, const olc::vf2d& center = { 0.0f, 0.0f }, const olc::vf2d& scale = { 1.0f,1.0f }, const olc::Pixel& tint = olc::WHITE);
        void DrawPartialRotatedDecal(const olc::vf2d& pos, olc::Decal* decal, const float fAngle, const olc::vf2d& center, const olc::vf2d& source_pos, const olc::vf2d& source_size, const olc::vf2d& scale = { 1.0f, 1.0f }, const olc::Pixel& tint = olc::WHITE);
        // Draws a multiline string as a decal, with tiniting and scaling
        void DrawStringDecal(const olc::vf2d& pos, const std::string& sText, const Pixel col = olc::WHITE, const olc::vf2d& scale = { 1.0f, 1.0f });
        void DrawStringPropDecal(const olc::vf2d& pos, const std::string& sText, const Pixel col = olc::WHITE, const olc::vf2d& scale = { 1.0f, 1.0f });
        // Draws a single shaded filled rectangle as a decal
        void DrawRectDecal(const olc::vf2d& pos, const olc::vf2d& size, const olc::Pixel col = olc::WHITE);
        void FillRectDecal(const olc::vf2d& pos, const olc::vf2d& size, const olc::Pixel col = olc::WHITE);
        // Draws a corner shaded rectangle as a decal
        void GradientFillRectDecal(const olc::vf2d& pos, const olc::vf2d& size, const olc::Pixel colTL, const olc::Pixel colBL, const olc::Pixel colBR, const olc::Pixel colTR);
        // Draws an arbitrary convex textured polygon using GPU
        void DrawPolygonDecal(olc::Decal* decal, const std::vector<olc::vf2d>& pos, const std::vector<olc::vf2d>& uv, const olc::Pixel tint = olc::WHITE);
        void DrawPolygonDecal(olc::Decal* decal, const std::vector<olc::vf2d>& pos, const std::vector<float>& depth, const std::vector<olc::vf2d>& uv, const olc::Pixel tint = olc::WHITE);
        void DrawPolygonDecal(olc::Decal* decal, const std::vector<olc::vf2d>& pos, const std::vector<olc::vf2d>& uv, const std::vector<olc::Pixel>& tint);
        void DrawPolygonDecal(olc::Decal* decal, const std::vector<olc::vf2d>& pos, const std::vector<olc::vf2d>& uv, const std::vector<olc::Pixel>& colours, const olc::Pixel tint);
        // Draws a line in Decal Space
        void DrawLineDecal(const olc::vf2d& pos1, const olc::vf2d& pos2, Pixel p = olc::WHITE);
        void DrawRotatedStringDecal(const olc::vf2d& pos, const std::string& sText, const float fAngle, const olc::vf2d& center = { 0.0f, 0.0f }, const olc::Pixel col = olc::WHITE, const olc::vf2d& scale = { 1.0f, 1.0f });
        void DrawRotatedStringPropDecal(const olc::vf2d& pos, const std::string& sText, const float fAngle, const olc::vf2d& center = { 0.0f, 0.0f }, const olc::Pixel col = olc::WHITE, const olc::vf2d& scale = { 1.0f, 1.0f });
        // Clears entire draw target to Pixel
        void Clear(Pixel p);
        // Clears the rendering back buffer
        void ClearBuffer(Pixel p, bool bDepth = true);
        // Returns the font image
        olc::Sprite* GetFontSprite();

        // Clip a line segment to visible area
        bool ClipLineToScreen(olc::vi2d& in_p1, olc::vi2d& in_p2);

        // Dont allow PGE to mark layers as dirty, so pixel graphics don't update
        void EnablePixelTransfer(const bool bEnable = true);

        // Command Console Routines
        void ConsoleShow(const olc::Key &keyExit, bool bSuspendTime = true);
        bool IsConsoleShowing() const;
        void ConsoleClear();
        std::stringstream& ConsoleOut();
        void ConsoleCaptureStdOut(const bool bCapture);

        // Text Entry Routines
        void TextEntryEnable(const bool bEnable, const std::string& sText = "");
        std::string TextEntryGetString() const;
        int32_t TextEntryGetCursor() const;
        bool IsTextEntryEnabled() const;



    private:
        void UpdateTextEntry();
        void UpdateConsole();

    public:

        // Experimental Lightweight 3D Routines ================
#ifdef OLC_ENABLE_EXPERIMENTAL
        // Set Manual View Matrix
		void LW3D_View(const std::array<float, 16>& m);
		// Set Manual World Matrix
		void LW3D_World(const std::array<float, 16>& m);
		// Set Manual Projection Matrix
		void LW3D_Projection(const std::array<float, 16>& m);
		
		// Draws a vector of vertices, interprted as individual triangles
		void LW3D_DrawTriangles(olc::Decal* decal, const std::vector<std::array<float,3>>& pos, const std::vector<olc::vf2d>& tex, const std::vector<olc::Pixel>& col);

		void LW3D_ModelTranslate(const float x, const float y, const float z);
		
		// Camera convenience functions
		void LW3D_SetCameraAtTarget(const float fEyeX, const float fEyeY, const float fEyeZ,
			const float fTargetX, const float fTargetY, const float fTargetZ,
			const float fUpX = 0.0f, const float fUpY = 1.0f, const float fUpZ = 0.0f);
		void LW3D_SetCameraAlongDirection(const float fEyeX, const float fEyeY, const float fEyeZ,
			const float fDirX, const float fDirY, const float fDirZ,
			const float fUpX = 0.0f, const float fUpY = 1.0f, const float fUpZ = 0.0f);

		// 3D Rendering Flags
		void LW3D_EnableDepthTest(const bool bEnableDepth);
		void LW3D_EnableBackfaceCulling(const bool bEnableCull);
#endif
    public: // Branding
        std::string sAppName;

    private: // Inner mysterious workings
        olc::Sprite*     pDrawTarget = nullptr;
        Pixel::Mode	nPixelMode = Pixel::NORMAL;
        float		fBlendFactor = 1.0f;
        olc::vi2d	vScreenSize = { 256, 240 };
        olc::vf2d	vInvScreenSize = { 1.0f / 256.0f, 1.0f / 240.0f };
        olc::vi2d	vPixelSize = { 4, 4 };
        olc::vi2d   vScreenPixelSize = { 4, 4 };
        olc::vi2d	vMousePos = { 0, 0 };
        int32_t		nMouseWheelDelta = 0;
        olc::vi2d	vMousePosCache = { 0, 0 };
        olc::vi2d   vMouseWindowPos = { 0, 0 };
        int32_t		nMouseWheelDeltaCache = 0;
        olc::vi2d	vWindowSize = { 0, 0 };
        olc::vi2d	vViewPos = { 0, 0 };
        olc::vi2d	vViewSize = { 0,0 };
        bool		bFullScreen = false;
        olc::vf2d	vPixel = { 1.0f, 1.0f };
        bool		bHasInputFocus = false;
        bool		bHasMouseFocus = false;
        bool		bEnableVSYNC = false;
        float		fFrameTimer = 1.0f;
        float		fLastElapsed = 0.0f;
        int			nFrameCount = 0;
        bool bSuspendTextureTransfer = false;
        Renderable  fontRenderable;
        std::vector<LayerDesc> vLayers;
        uint8_t		nTargetLayer = 0;
        uint32_t	nLastFPS = 0;
        bool        bPixelCohesion = false;
        DecalMode   nDecalMode = DecalMode::NORMAL;
        DecalStructure nDecalStructure = DecalStructure::FAN;
        std::function<olc::Pixel(const int x, const int y, const olc::Pixel&, const olc::Pixel&)> funcPixelMode;
        std::chrono::time_point<std::chrono::system_clock> m_tp1, m_tp2;
        std::vector<olc::vi2d> vFontSpacing;
        std::vector<std::string> vDroppedFiles;
        std::vector<std::string> vDroppedFilesCache;
        olc::vi2d vDroppedFilesPoint;
        olc::vi2d vDroppedFilesPointCache;

        // Command Console Specific
        bool bConsoleShow = false;
        bool bConsoleSuspendTime = false;
        olc::Key keyConsoleExit = olc::Key::F1;
        std::stringstream ssConsoleOutput;
        std::streambuf* sbufOldCout = nullptr;
        olc::vi2d vConsoleSize;
        olc::vi2d vConsoleCursor = { 0,0 };
        olc::vf2d vConsoleCharacterScale = { 1.0f, 2.0f };
        std::vector<std::string> sConsoleLines;
        std::list<std::string> sCommandHistory;
        std::list<std::string>::iterator sCommandHistoryIt;

        // Text Entry Specific
        bool bTextEntryEnable = false;
        std::string sTextEntryString = "";
        int32_t nTextEntryCursor = 0;
        std::vector<std::tuple<olc::Key, std::string, std::string>> vKeyboardMap;



        // State of keyboard
        bool		pKeyNewState[256] = { 0 };
        bool		pKeyOldState[256] = { 0 };
        HWButton	pKeyboardState[256] = { 0 };

        // State of mouse
        bool		pMouseNewState[nMouseButtons] = { 0 };
        bool		pMouseOldState[nMouseButtons] = { 0 };
        HWButton	pMouseState[nMouseButtons] = { 0 };

        // The main engine thread
        void		EngineThread();


        // If anything sets this flag to false, the engine
        // "should" shut down gracefully
        static std::atomic<bool> bAtomActive;

    public:
        // "Break In" Functions
        void olc_UpdateMouse(int32_t x, int32_t y);
        void olc_UpdateMouseWheel(int32_t delta);
        void olc_UpdateWindowSize(int32_t x, int32_t y);
        void olc_UpdateViewport();
        void olc_ConstructFontSheet();
        void olc_CoreUpdate();
        void olc_PrepareEngine();
        void olc_UpdateMouseState(int32_t button, bool state);
        void olc_UpdateKeyState(int32_t key, bool state);
        void olc_UpdateMouseFocus(bool state);
        void olc_UpdateKeyFocus(bool state);
        void olc_Terminate();
        void olc_DropFiles(int32_t x, int32_t y, const std::vector<std::string>& vFiles);
        void olc_Reanimate();
        bool olc_IsRunning();

        // At the very end of this file, chooses which
        // components to compile
        virtual void olc_ConfigureSystem();

        // NOTE: Items Here are to be deprecated, I have left them in for now
        // in case you are using them, but they will be removed.
        // olc::vf2d	vSubPixelOffset = { 0.0f, 0.0f };

    public: // PGEX Stuff
        friend class PGEX;
        void pgex_Register(olc::PGEX* pgex);

    private:
        std::vector<olc::PGEX*> vExtensions;
    };



    // O------------------------------------------------------------------------------O
    // | PGE EXTENSION BASE CLASS - Permits access to PGE functions from extension    |
    // O------------------------------------------------------------------------------O
    class PGEX
    {
        friend class olc::PixelGameEngine;
    public:
        PGEX(bool bHook = false);

    protected:
        virtual void OnBeforeUserCreate();
        virtual void OnAfterUserCreate();
        virtual bool OnBeforeUserUpdate(float &fElapsedTime);
        virtual void OnAfterUserUpdate(float fElapsedTime);

    protected:
        static PixelGameEngine* pge;
    };
}

#pragma endregion

#endif // OLC_PGE_DEF


// O------------------------------------------------------------------------------O
// | START OF OLC_PGE_APPLICATION                                                 |
// O------------------------------------------------------------------------------O
//#ifdef OLC_PGE_APPLICATION
//#undef OLC_PGE_APPLICATION

// O------------------------------------------------------------------------------O
// | olcPixelGameEngine INTERFACE IMPLEMENTATION (CORE)                           |
// | Note: The core implementation is platform independent                        |
// O------------------------------------------------------------------------------O

// O------------------------------------------------------------------------------O
// | olcPixelGameEngine Renderers - the draw-y bits                               |
// O------------------------------------------------------------------------------O

#if !defined(OLC_PGE_HEADLESS)

#pragma region renderer_ogl33
// O------------------------------------------------------------------------------O
// | START RENDERER: OpenGL 3.3 (3.0 es) (sh-sh-sh-shaders....)                   |
// O------------------------------------------------------------------------------O
#if defined(OLC_GFX_OPENGL33)

#if defined(OLC_PLATFORM_WINAPI)
	#include <dwmapi.h>
	//#include <gl/GL.h>
	#if !defined(__MINGW32__)
		#pragma comment(lib, "Dwmapi.lib")
	#endif		
	//typedef void __stdcall locSwapInterval_t(GLsizei n);
	typedef HDC glDeviceContext_t;
	typedef HGLRC glRenderContext_t;
	//#define CALLSTYLE __stdcall
	#define OGL_LOAD(t, n) (t*)wglGetProcAddress(#n)
#endif
//
//#if defined(__linux__) || defined(__FreeBSD__)
//	#include <GL/gl.h>
//#endif

#if defined(OLC_PLATFORM_X11)
	/*namespace X11
	{
		#include <GL/glx.h>
	}
	typedef int(locSwapInterval_t)(X11::Display* dpy, X11::GLXDrawable drawable, int interval);*/
	typedef X11::GLXContext glDeviceContext_t;
	typedef X11::GLXContext glRenderContext_t;
	//#define CALLSTYLE 
	#define OGL_LOAD(t, n) (t*)glXGetProcAddress((unsigned char*)#n);
#endif

//#if defined(__APPLE__)
//	#define GL_SILENCE_DEPRECATION
//	#include <OpenGL/OpenGL.h>
//	#include <OpenGL/gl.h>
//	#include <OpenGL/glu.h>
//#endif

#if defined(OLC_PLATFORM_EMSCRIPTEN)
	#include <EGL/egl.h>
	#include <GLES2/gl2.h>
	#define GL_GLEXT_PROTOTYPES
	#include <GLES2/gl2ext.h>
	#include <emscripten/emscripten.h>
	#define CALLSTYLE
	typedef EGLBoolean(locSwapInterval_t)(EGLDisplay display, EGLint interval);
	#define GL_CLAMP GL_CLAMP_TO_EDGE
	#define OGL_LOAD(t, n) n;
#endif

namespace olc
{
//	typedef char GLchar;
//	typedef ptrdiff_t GLsizeiptr;
//	typedef GLuint CALLSTYLE locCreateShader_t(GLenum type);
//	typedef GLuint CALLSTYLE locCreateProgram_t(void);
//	typedef void CALLSTYLE locDeleteShader_t(GLuint shader);
//#if defined(OLC_PLATFORM_EMSCRIPTEN)
//	typedef void CALLSTYLE locShaderSource_t(GLuint shader, GLsizei count, const GLchar* const* string, const GLint* length);
//#else
//	typedef void CALLSTYLE locShaderSource_t(GLuint shader, GLsizei count, const GLchar** string, const GLint* length);
//#endif
//	typedef void CALLSTYLE locCompileShader_t(GLuint shader);
//	typedef void CALLSTYLE locLinkProgram_t(GLuint program);
//	typedef void CALLSTYLE locDeleteProgram_t(GLuint program);
//	typedef void CALLSTYLE locAttachShader_t(GLuint program, GLuint shader);
//	typedef void CALLSTYLE locBindBuffer_t(GLenum target, GLuint buffer);
//	typedef void CALLSTYLE locBufferData_t(GLenum target, GLsizeiptr size, const void* data, GLenum usage);
//	typedef void CALLSTYLE locGenBuffers_t(GLsizei n, GLuint* buffers);
//	typedef void CALLSTYLE locVertexAttribPointer_t(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void* pointer);
//	typedef void CALLSTYLE locEnableVertexAttribArray_t(GLuint index);
//	typedef void CALLSTYLE locUseProgram_t(GLuint program);
//	typedef void CALLSTYLE locBindVertexArray_t(GLuint array);
//	typedef void CALLSTYLE locGenVertexArrays_t(GLsizei n, GLuint* arrays);
//	typedef void CALLSTYLE locGetShaderInfoLog_t(GLuint shader, GLsizei bufSize, GLsizei* length, GLchar* infoLog);
//	typedef GLint CALLSTYLE locGetUniformLocation_t(GLuint program, const GLchar* name);
//	typedef void CALLSTYLE locUniform1f_t(GLint location, GLfloat v0);
//	typedef void CALLSTYLE locUniform1i_t(GLint location, GLint v0);
//	typedef void CALLSTYLE locUniform2fv_t(GLint location, GLsizei count, const GLfloat* value);
//	typedef void CALLSTYLE locActiveTexture_t(GLenum texture);
//	typedef void CALLSTYLE locGenFrameBuffers_t(GLsizei n, GLuint* ids);
//	typedef void CALLSTYLE locBindFrameBuffer_t(GLenum target, GLuint fb);
//	typedef GLenum CALLSTYLE locCheckFrameBufferStatus_t(GLenum target);
//	typedef void CALLSTYLE locDeleteFrameBuffers_t(GLsizei n, const GLuint* fbs);
//	typedef void CALLSTYLE locFrameBufferTexture2D_t(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level);
//	typedef void CALLSTYLE locDrawBuffers_t(GLsizei n, const GLenum* bufs);
//	typedef void CALLSTYLE locBlendFuncSeparate_t(GLenum srcRGB, GLenum dstRGB, GLenum srcAlpha, GLenum dstAlpha);

	

	class Renderer_OGL33 : public olc::Renderer
	{
	private:
#if defined(OLC_PLATFORM_EMSCRIPTEN)
		EGLDisplay olc_Display;
		EGLConfig olc_Config;
		EGLContext olc_Context;
		EGLSurface olc_Surface;
#endif

#if defined(OLC_PLATFORM_GLUT)
		bool mFullScreen = false;
#else
	#if !defined(OLC_PLATFORM_EMSCRIPTEN)
		glDeviceContext_t glDeviceContext = 0;
		glRenderContext_t glRenderContext = 0;
	#endif
#endif
		bool bSync = false;
		olc::DecalMode nDecalMode = olc::DecalMode(-1); // Thanks Gusgo & Bispoo
#if defined(OLC_PLATFORM_X11)
		X11::Display* olc_Display = nullptr;
		X11::Window* olc_Window = nullptr;
		X11::XVisualInfo* olc_VisualInfo = nullptr;
#endif

	private:
		locCreateShader_t* locCreateShader = nullptr;
		locShaderSource_t* locShaderSource = nullptr;
		locCompileShader_t* locCompileShader = nullptr;
		locDeleteShader_t* locDeleteShader = nullptr;
		locCreateProgram_t* locCreateProgram = nullptr;
		locDeleteProgram_t* locDeleteProgram = nullptr;
		locLinkProgram_t* locLinkProgram = nullptr;
		locAttachShader_t* locAttachShader = nullptr;
		locBindBuffer_t* locBindBuffer = nullptr;
		locBufferData_t* locBufferData = nullptr;
		locGenBuffers_t* locGenBuffers = nullptr;
		locVertexAttribPointer_t* locVertexAttribPointer = nullptr;
		locEnableVertexAttribArray_t* locEnableVertexAttribArray = nullptr;
		locUseProgram_t* locUseProgram = nullptr;
		locBindVertexArray_t* locBindVertexArray = nullptr;
		locGenVertexArrays_t* locGenVertexArrays = nullptr;
		locSwapInterval_t* locSwapInterval = nullptr;
		locGetShaderInfoLog_t* locGetShaderInfoLog = nullptr;

		uint32_t m_nFS = 0;
		uint32_t m_nVS = 0;
		uint32_t m_nQuadShader = 0;
		uint32_t m_vbQuad = 0;
		uint32_t m_vaQuad = 0;

		struct locVertex
		{
			float pos[3];
			olc::vf2d tex;
			olc::Pixel col;
		};

		locVertex pVertexMem[OLC_MAX_VERTS];

		olc::Renderable rendBlankQuad;

	public:
		void PrepareDevice() override
		{
#if defined(OLC_PLATFORM_GLUT)
			//glutInit has to be called with main() arguments, make fake ones
			int argc = 0;
			char* argv[1] = { (char*)"" };
			glutInit(&argc, argv);
			glutInitWindowPosition(0, 0);
			glutInitWindowSize(512, 512);
			glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH | GLUT_RGBA);
			// Creates the window and the OpenGL context for it
			glutCreateWindow("OneLoneCoder.com - Pixel Game Engine");
			glEnable(GL_TEXTURE_2D); // Turn on texturing
			glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
#endif
		}

		olc::rcode CreateDevice(std::vector<void*> params, bool bFullScreen, bool bVSYNC) override
		{
			// Create OpenGL Context
#if defined(OLC_PLATFORM_WINAPI)
			// Create Device Context
			glDeviceContext = GetDC((HWND)(params[0]));
			PIXELFORMATDESCRIPTOR pfd =
			{
				sizeof(PIXELFORMATDESCRIPTOR), 1,
				PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
				PFD_TYPE_RGBA, 32, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
				PFD_MAIN_PLANE, 0, 0, 0, 0
			};

			int pf = 0;
			if (!(pf = ChoosePixelFormat(glDeviceContext, &pfd))) return olc::FAIL;
			SetPixelFormat(glDeviceContext, pf, &pfd);

			if (!(glRenderContext = wglCreateContext(glDeviceContext))) return olc::FAIL;
			wglMakeCurrent(glDeviceContext, glRenderContext);

			// Set Vertical Sync
			locSwapInterval = OGL_LOAD(locSwapInterval_t, wglSwapIntervalEXT);
			if (locSwapInterval && !bVSYNC) locSwapInterval(0);
			bSync = bVSYNC;
#endif

#if defined(OLC_PLATFORM_X11)
			using namespace X11;
			// Linux has tighter coupling between OpenGL and X11, so we store
			// various "platform" handles in the renderer
			olc_Display = (X11::Display*)(params[0]);
			olc_Window = (X11::Window*)(params[1]);
			olc_VisualInfo = (X11::XVisualInfo*)(params[2]);

			glDeviceContext = glXCreateContext(olc_Display, olc_VisualInfo, nullptr, GL_TRUE);
			glXMakeCurrent(olc_Display, *olc_Window, glDeviceContext);

			XWindowAttributes gwa;
			XGetWindowAttributes(olc_Display, *olc_Window, &gwa);
			glViewport(0, 0, gwa.width, gwa.height);

			locSwapInterval = OGL_LOAD(locSwapInterval_t, glXSwapIntervalEXT);

			if (locSwapInterval == nullptr && !bVSYNC)
			{
				printf("NOTE: Could not disable VSYNC, glXSwapIntervalEXT() was not found!\n");
				printf("      Don't worry though, things will still work, it's just the\n");
				printf("      frame rate will be capped to your monitors refresh rate - javidx9\n");
			}

			if (locSwapInterval != nullptr && !bVSYNC)
				locSwapInterval(olc_Display, *olc_Window, 0);
#endif		

#if defined(OLC_PLATFORM_EMSCRIPTEN)
			EGLint const attribute_list[] = { EGL_RED_SIZE, 8, EGL_GREEN_SIZE, 8, EGL_BLUE_SIZE, 8, EGL_ALPHA_SIZE, 8, EGL_NONE };
			EGLint const context_config[] = { EGL_CONTEXT_CLIENT_VERSION , 2, EGL_NONE };
			EGLint num_config;

			olc_Display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
			eglInitialize(olc_Display, nullptr, nullptr);
			eglChooseConfig(olc_Display, attribute_list, &olc_Config, 1, &num_config);

			/* create an EGL rendering context */
			olc_Context = eglCreateContext(olc_Display, olc_Config, EGL_NO_CONTEXT, context_config);
			olc_Surface = eglCreateWindowSurface(olc_Display, olc_Config, NULL, nullptr);
			eglMakeCurrent(olc_Display, olc_Surface, olc_Surface, olc_Context);
			//eglSwapInterval is currently a NOP, plement anyways in case it becomes supported
			locSwapInterval = &eglSwapInterval;
			locSwapInterval(olc_Display, bVSYNC ? 1 : 0);
#endif

#if defined(OLC_PLATFORM_GLUT)
			mFullScreen = bFullScreen;
			if (!bVSYNC)
			{
#if defined(__APPLE__)
				GLint sync = 0;
				CGLContextObj ctx = CGLGetCurrentContext();
				if (ctx) CGLSetParameter(ctx, kCGLCPSwapInterval, &sync);
#endif
			}
#else
	#if !defined(OLC_PLATFORM_EMSCRIPTEN)
			glEnable(GL_TEXTURE_2D); // Turn on texturing
			glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
	#endif
#endif
			// Load External OpenGL Functions
			locCreateShader = OGL_LOAD(locCreateShader_t, glCreateShader);
			locCompileShader = OGL_LOAD(locCompileShader_t, glCompileShader);
			locShaderSource = OGL_LOAD(locShaderSource_t, glShaderSource);
			locDeleteShader = OGL_LOAD(locDeleteShader_t, glDeleteShader);
			locCreateProgram = OGL_LOAD(locCreateProgram_t, glCreateProgram);
			locDeleteProgram = OGL_LOAD(locDeleteProgram_t, glDeleteProgram);
			locLinkProgram = OGL_LOAD(locLinkProgram_t, glLinkProgram);
			locAttachShader = OGL_LOAD(locAttachShader_t, glAttachShader);
			locBindBuffer = OGL_LOAD(locBindBuffer_t, glBindBuffer);
			locBufferData = OGL_LOAD(locBufferData_t, glBufferData);
			locGenBuffers = OGL_LOAD(locGenBuffers_t, glGenBuffers);
			locVertexAttribPointer = OGL_LOAD(locVertexAttribPointer_t, glVertexAttribPointer);
			locEnableVertexAttribArray = OGL_LOAD(locEnableVertexAttribArray_t, glEnableVertexAttribArray);
			locUseProgram = OGL_LOAD(locUseProgram_t, glUseProgram);
			locGetShaderInfoLog = OGL_LOAD(locGetShaderInfoLog_t, glGetShaderInfoLog);
#if !defined(OLC_PLATFORM_EMSCRIPTEN)
			locBindVertexArray = OGL_LOAD(locBindVertexArray_t, glBindVertexArray);
			locGenVertexArrays = OGL_LOAD(locGenVertexArrays_t, glGenVertexArrays);
#else
			locBindVertexArray = glBindVertexArrayOES;
			locGenVertexArrays = glGenVertexArraysOES;
#endif

			// Load & Compile Quad Shader - assumes no errors
			m_nFS = locCreateShader(0x8B30);
			const GLchar* strFS =
#if defined(__arm__) || defined(OLC_PLATFORM_EMSCRIPTEN)
				"#version 300 es\n"
				"precision mediump float;"
#else
				"#version 330 core\n"
#endif
				"out vec4 pixel;\n""in vec2 oTex;\n"
				"in vec4 oCol;\n""uniform sampler2D sprTex;\n""void main(){pixel = texture(sprTex, oTex) * oCol;}";
			locShaderSource(m_nFS, 1, &strFS, NULL);
			locCompileShader(m_nFS);

			m_nVS = locCreateShader(0x8B31);
			const GLchar* strVS =
#if defined(__arm__) || defined(OLC_PLATFORM_EMSCRIPTEN)
				"#version 300 es\n"
				"precision mediump float;"
#else
				"#version 330 core\n"
#endif
				"layout(location = 0) in vec3 aPos;\n""layout(location = 1) in vec2 aTex;\n"
				"layout(location = 2) in vec4 aCol;\n""out vec2 oTex;\n""out vec4 oCol;\n"
				"void main(){ float p = 1.0 / aPos.z; gl_Position = p * vec4(aPos.x, aPos.y, 0.0, 1.0); oTex = p * aTex; oCol = aCol;}";
			locShaderSource(m_nVS, 1, &strVS, NULL);
			locCompileShader(m_nVS);

			m_nQuadShader = locCreateProgram();
			locAttachShader(m_nQuadShader, m_nFS);
			locAttachShader(m_nQuadShader, m_nVS);
			locLinkProgram(m_nQuadShader);

			// Create Quad
			locGenBuffers(1, &m_vbQuad);
			locGenVertexArrays(1, &m_vaQuad);
			locBindVertexArray(m_vaQuad);
			locBindBuffer(0x8892, m_vbQuad);

			locVertex verts[OLC_MAX_VERTS];
			locBufferData(0x8892, sizeof(locVertex) * OLC_MAX_VERTS, verts, 0x88E0);
			locVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(locVertex), 0); locEnableVertexAttribArray(0);
			locVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(locVertex), (void*)(3 * sizeof(float))); locEnableVertexAttribArray(1);
			locVertexAttribPointer(2, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(locVertex), (void*)(5 * sizeof(float)));	locEnableVertexAttribArray(2);
			locBindBuffer(0x8892, 0);
			locBindVertexArray(0);

			// Create blank texture for spriteless decals
			rendBlankQuad.Create(1, 1);
			rendBlankQuad.Sprite()->GetData()[0] = olc::WHITE;
			rendBlankQuad.Decal()->Update();
			return olc::rcode::OK;
		}

		olc::rcode DestroyDevice() override
		{
#if defined(OLC_PLATFORM_WINAPI)
			wglDeleteContext(glRenderContext);
#endif

#if defined(OLC_PLATFORM_X11)
			glXMakeCurrent(olc_Display, None, NULL);
			glXDestroyContext(olc_Display, glDeviceContext);
#endif

#if defined(OLC_PLATFORM_GLUT)
			glutDestroyWindow(glutGetWindow());
#endif

#if defined(OLC_PLATFORM_EMSCRIPTEN)
			eglMakeCurrent(olc_Display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
			eglDestroyContext(olc_Display, olc_Context);
			eglDestroySurface(olc_Display, olc_Surface);
			eglTerminate(olc_Display);
			olc_Display = EGL_NO_DISPLAY;
			olc_Surface = EGL_NO_SURFACE;
			olc_Context = EGL_NO_CONTEXT;
#endif
			return olc::rcode::OK;
		}

		void DisplayFrame() override
		{
#if defined(OLC_PLATFORM_WINAPI)
			SwapBuffers(glDeviceContext);
			if (bSync) DwmFlush(); // Woooohooooooo!!!! SMOOOOOOOTH!
#endif	

#if defined(OLC_PLATFORM_X11)
			X11::glXSwapBuffers(olc_Display, *olc_Window);
#endif		

#if defined(OLC_PLATFORM_GLUT)
			glutSwapBuffers();
#endif

#if defined(OLC_PLATFORM_EMSCRIPTEN)
			eglSwapBuffers(olc_Display, olc_Surface);
#endif
		}

		void PrepareDrawing() override
		{
			glEnable(GL_BLEND);
			nDecalMode = DecalMode::NORMAL;
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			locUseProgram(m_nQuadShader);
			locBindVertexArray(m_vaQuad);

#if defined(OLC_PLATFORM_EMSCRIPTEN)
			locVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(locVertex), 0); locEnableVertexAttribArray(0);
			locVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(locVertex), (void*)(3 * sizeof(float))); locEnableVertexAttribArray(1);
			locVertexAttribPointer(2, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(locVertex), (void*)(5 * sizeof(float)));	locEnableVertexAttribArray(2);
#endif
		}

		void SetDecalMode(const olc::DecalMode& mode) override
		{
			if (mode != nDecalMode)
			{
				switch (mode)
				{
				case olc::DecalMode::NORMAL: glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);	break;
				case olc::DecalMode::ADDITIVE: glBlendFunc(GL_SRC_ALPHA, GL_ONE); break;
				case olc::DecalMode::MULTIPLICATIVE: glBlendFunc(GL_DST_COLOR, GL_ONE_MINUS_SRC_ALPHA);	break;
				case olc::DecalMode::STENCIL: glBlendFunc(GL_ZERO, GL_SRC_ALPHA); break;
				case olc::DecalMode::ILLUMINATE: glBlendFunc(GL_ONE_MINUS_SRC_ALPHA, GL_SRC_ALPHA);	break;
				case olc::DecalMode::WIREFRAME: glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);	break;
				}

				nDecalMode = mode;
			}
		}

		void DrawLayerQuad(const olc::vf2d& offset, const olc::vf2d& scale, const olc::Pixel tint) override
		{
			locBindBuffer(0x8892, m_vbQuad);
			locVertex verts[4] = {
				{{-1.0f, -1.0f, 1.0}, {0.0f * scale.x + offset.x, 1.0f * scale.y + offset.y}, tint},
				{{+1.0f, -1.0f, 1.0}, {1.0f * scale.x + offset.x, 1.0f * scale.y + offset.y}, tint},
				{{-1.0f, +1.0f, 1.0}, {0.0f * scale.x + offset.x, 0.0f * scale.y + offset.y}, tint},
				{{+1.0f, +1.0f, 1.0}, {1.0f * scale.x + offset.x, 0.0f * scale.y + offset.y}, tint},
			};

			locBufferData(0x8892, sizeof(locVertex) * 4, verts, 0x88E0);
			glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		}

		void DrawDecal(const olc::DecalInstance& decal) override
		{
			SetDecalMode(decal.mode);
			if (decal.decal == nullptr)
				glBindTexture(GL_TEXTURE_2D, rendBlankQuad.Decal()->id);
			else
				glBindTexture(GL_TEXTURE_2D, decal.decal->id);

			locBindBuffer(0x8892, m_vbQuad);

			for (uint32_t i = 0; i < decal.points; i++)
				pVertexMem[i] = { { decal.pos[i].x, decal.pos[i].y, decal.w[i] }, { decal.uv[i].x, decal.uv[i].y }, decal.tint[i] };

			locBufferData(0x8892, sizeof(locVertex) * decal.points, pVertexMem, 0x88E0);

			if (nDecalMode == DecalMode::WIREFRAME)
				glDrawArrays(GL_LINE_LOOP, 0, decal.points);
			else
			{
				if (decal.structure == olc::DecalStructure::FAN)
					glDrawArrays(GL_TRIANGLE_FAN, 0, decal.points);
				else if (decal.structure == olc::DecalStructure::STRIP)
					glDrawArrays(GL_TRIANGLE_STRIP, 0, decal.points);
				else if (decal.structure == olc::DecalStructure::LIST)
					glDrawArrays(GL_TRIANGLES, 0, decal.points);
			}
		}

		uint32_t CreateTexture(const uint32_t width, const uint32_t height, const bool filtered, const bool clamp) override
		{
			UNUSED(width);
			UNUSED(height);
			uint32_t id = 0;
			glGenTextures(1, &id);
			glBindTexture(GL_TEXTURE_2D, id);

			if (filtered)
			{
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			}
			else
			{
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			}

			if (clamp)
			{
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
			}
			else
			{
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			}
#if !defined(OLC_PLATFORM_EMSCRIPTEN)
			glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
#endif
			return id;
		}

		uint32_t DeleteTexture(const uint32_t id) override
		{
			glDeleteTextures(1, &id);
			return id;
		}

		void UpdateTexture(uint32_t id, olc::Sprite* spr) override
		{
			UNUSED(id);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, spr->width, spr->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, spr->GetData());
		}

		void ReadTexture(uint32_t id, olc::Sprite* spr) override
		{
			glReadPixels(0, 0, spr->width, spr->height, GL_RGBA, GL_UNSIGNED_BYTE, spr->GetData());
		}

		void ApplyTexture(uint32_t id) override
		{
			glBindTexture(GL_TEXTURE_2D, id);
		}

		void ClearBuffer(olc::Pixel p, bool bDepth) override
		{
			glClearColor(float(p.r) / 255.0f, float(p.g) / 255.0f, float(p.b) / 255.0f, float(p.a) / 255.0f);
			glClear(GL_COLOR_BUFFER_BIT);
			if (bDepth) glClear(GL_DEPTH_BUFFER_BIT);
		}

		void UpdateViewport(const olc::vi2d& pos, const olc::vi2d& size) override
		{
			glViewport(pos.x, pos.y, size.x, size.y);
		}
	};
}
#endif
// O------------------------------------------------------------------------------O
// | END RENDERER: OpenGL 3.3 (3.0 es) (sh-sh-sh-shaders....)                     |
// O------------------------------------------------------------------------------O
#pragma endregion

// O------------------------------------------------------------------------------O
// | olcPixelGameEngine Image loaders                                             |
// O------------------------------------------------------------------------------O

#pragma region image_gdi
// O------------------------------------------------------------------------------O
// | START IMAGE LOADER: GDI+, Windows Only, always exists, a little slow         |
// O------------------------------------------------------------------------------O
#if defined(OLC_IMAGE_GDI)

#define min(a, b) ((a < b) ? a : b)
#define max(a, b) ((a > b) ? a : b)
#include <objidl.h>
#include <gdiplus.h>
#if defined(__MINGW32__) // Thanks Gusgo & Dandistine, but c'mon mingw!! wtf?!
	#include <gdiplus/gdiplusinit.h>
#else
	#include <gdiplusinit.h>
#endif
#include <shlwapi.h>
#undef min
#undef max

#if !defined(__MINGW32__)
	#pragma comment(lib, "gdiplus.lib")
	#pragma comment(lib, "Shlwapi.lib")
#endif

namespace olc
{
	// Thanks @MaGetzUb for this, which allows sprites to be defined
	// at construction, by initialising the GDI subsystem
	static class GDIPlusStartup
	{
	public:
		GDIPlusStartup()
		{			
			Gdiplus::GdiplusStartupInput startupInput;
			GdiplusStartup(&token, &startupInput, NULL);
		}

		ULONG_PTR	token;
		
		~GDIPlusStartup()
		{
			// Well, MarcusTU thought this was important :D
			Gdiplus::GdiplusShutdown(token);
		}
	} gdistartup;

	class ImageLoader_GDIPlus : public olc::ImageLoader
	{
	private:
		std::wstring ConvertS2W(std::string s)
		{
#ifdef __MINGW32__
			wchar_t* buffer = new wchar_t[s.length() + 1];
			mbstowcs(buffer, s.c_str(), s.length());
			buffer[s.length()] = L'\0';
#else
			int count = MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, NULL, 0);
			wchar_t* buffer = new wchar_t[count];
			MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, buffer, count);
#endif
			std::wstring w(buffer);
			delete[] buffer;
			return w;
		}

	public:
		ImageLoader_GDIPlus() : ImageLoader()
		{}

		olc::rcode LoadImageResource(olc::Sprite* spr, const std::string& sImageFile, olc::ResourcePack* pack) override
		{
			// clear out existing sprite
			spr->pColData.clear();

			// Open file
			UNUSED(pack);
			Gdiplus::Bitmap* bmp = nullptr;
			if (pack != nullptr)
			{
				// Load sprite from input stream
				ResourceBuffer rb = pack->GetFileBuffer(sImageFile);
				bmp = Gdiplus::Bitmap::FromStream(SHCreateMemStream((BYTE*)rb.vMemory.data(), UINT(rb.vMemory.size())));
			}
			else
			{
				// Check file exists
				if (!_gfs::exists(sImageFile)) return olc::rcode::NO_FILE;

				// Load sprite from file
				bmp = Gdiplus::Bitmap::FromFile(ConvertS2W(sImageFile).c_str());
			}

			if (bmp->GetLastStatus() != Gdiplus::Ok) return olc::rcode::FAIL;
			spr->width = bmp->GetWidth();
			spr->height = bmp->GetHeight();

			spr->pColData.resize(spr->width * spr->height);

			for (int y = 0; y < spr->height; y++)
				for (int x = 0; x < spr->width; x++)
				{
					Gdiplus::Color c;
					bmp->GetPixel(x, y, &c);
					spr->SetPixel(x, y, olc::Pixel(c.GetRed(), c.GetGreen(), c.GetBlue(), c.GetAlpha()));
				}
			delete bmp;
			return olc::rcode::OK;
		}

		olc::rcode SaveImageResource(olc::Sprite* spr, const std::string& sImageFile) override
		{
			return olc::rcode::OK;
		}
	};
}
#endif
// O------------------------------------------------------------------------------O
// | END IMAGE LOADER: GDI+                                                       |
// O------------------------------------------------------------------------------O
#pragma endregion

#pragma region image_libpng

#undef OLC_IMAGE_LIBPNG
// O------------------------------------------------------------------------------O
// | START IMAGE LOADER: libpng, default on linux, requires -lpng  (libpng-dev)   |
// O------------------------------------------------------------------------------O
//#if defined(OLC_IMAGE_LIBPNG)
//#include <png.h>
//namespace olc
//{
//	void pngReadStream(png_structp pngPtr, png_bytep data, png_size_t length)
//	{
//		png_voidp a = png_get_io_ptr(pngPtr);
//		((std::istream*)a)->read((char*)data, length);
//	}
//
//	class ImageLoader_LibPNG : public olc::ImageLoader
//	{
//	public:
//		ImageLoader_LibPNG() : ImageLoader()
//		{}
//
//		olc::rcode LoadImageResource(olc::Sprite* spr, const std::string& sImageFile, olc::ResourcePack* pack) override
//		{
//			UNUSED(pack);
//
//			// clear out existing sprite
//			spr->pColData.clear();
//
//			////////////////////////////////////////////////////////////////////////////
//			// Use libpng, Thanks to Guillaume Cottenceau
//			// https://gist.github.com/niw/5963798
//			// Also reading png from streams
//			// http://www.piko3d.net/tutorials/libpng-tutorial-loading-png-files-from-streams/
//			png_structp png;
//			png_infop info;
//
//			auto loadPNG = [&]()
//			{
//				png_read_info(png, info);
//				png_byte color_type;
//				png_byte bit_depth;
//				png_bytep* row_pointers;
//				spr->width = png_get_image_width(png, info);
//				spr->height = png_get_image_height(png, info);
//				color_type = png_get_color_type(png, info);
//				bit_depth = png_get_bit_depth(png, info);
//				if (bit_depth == 16) png_set_strip_16(png);
//				if (color_type == PNG_COLOR_TYPE_PALETTE) png_set_palette_to_rgb(png);
//				if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8)	png_set_expand_gray_1_2_4_to_8(png);
//				if (png_get_valid(png, info, PNG_INFO_tRNS)) png_set_tRNS_to_alpha(png);
//				if (color_type == PNG_COLOR_TYPE_RGB || color_type == PNG_COLOR_TYPE_GRAY || color_type == PNG_COLOR_TYPE_PALETTE)
//					png_set_filler(png, 0xFF, PNG_FILLER_AFTER);
//				if (color_type == PNG_COLOR_TYPE_GRAY || color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
//					png_set_gray_to_rgb(png);
//				png_read_update_info(png, info);
//				row_pointers = (png_bytep*)malloc(sizeof(png_bytep) * spr->height);
//				for (int y = 0; y < spr->height; y++) {
//					row_pointers[y] = (png_byte*)malloc(png_get_rowbytes(png, info));
//				}
//				png_read_image(png, row_pointers);
//				////////////////////////////////////////////////////////////////////////////
//				// Create sprite array
//				spr->pColData.resize(spr->width * spr->height);
//				// Iterate through image rows, converting into sprite format
//				for (int y = 0; y < spr->height; y++)
//				{
//					png_bytep row = row_pointers[y];
//					for (int x = 0; x < spr->width; x++)
//					{
//						png_bytep px = &(row[x * 4]);
//						spr->SetPixel(x, y, Pixel(px[0], px[1], px[2], px[3]));
//					}
//				}
//
//				for (int y = 0; y < spr->height; y++) // Thanks maksym33
//					free(row_pointers[y]);
//				free(row_pointers);
//				png_destroy_read_struct(&png, &info, nullptr);
//			};
//
//			png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
//			if (!png) goto fail_load;
//
//			info = png_create_info_struct(png);
//			if (!info) goto fail_load;
//
//			if (setjmp(png_jmpbuf(png))) goto fail_load;
//
//			if (pack == nullptr)
//			{
//				FILE* f = fopen(sImageFile.c_str(), "rb");
//				if (!f) return olc::rcode::NO_FILE;
//				png_init_io(png, f);
//				loadPNG();
//				fclose(f);
//			}
//			else
//			{
//				ResourceBuffer rb = pack->GetFileBuffer(sImageFile);
//				std::istream is(&rb);
//				png_set_read_fn(png, (png_voidp)&is, pngReadStream);
//				loadPNG();
//			}
//
//			return olc::rcode::OK;
//
//		fail_load:
//			spr->width = 0;
//			spr->height = 0;
//			spr->pColData.clear();
//			return olc::rcode::FAIL;
//		}
//
//		olc::rcode SaveImageResource(olc::Sprite* spr, const std::string& sImageFile) override
//		{
//			return olc::rcode::OK;
//		}
//	};
//}
//#endif
// O------------------------------------------------------------------------------O
// | END IMAGE LOADER:                                                            |
// O------------------------------------------------------------------------------O
#pragma endregion


// O------------------------------------------------------------------------------O
// | olcPixelGameEngine Platforms                                                 |
// O------------------------------------------------------------------------------O

#pragma region platform_windows
// O------------------------------------------------------------------------------O
// | START PLATFORM: MICROSOFT WINDOWS XP, VISTA, 7, 8, 10                        |
// O------------------------------------------------------------------------------O
#if defined(OLC_PLATFORM_WINAPI)

#if defined(_WIN32) && !defined(__MINGW32__)
	#pragma comment(lib, "user32.lib")		// Visual Studio Only
	#pragma comment(lib, "gdi32.lib")		// For other Windows Compilers please add
	#pragma comment(lib, "opengl32.lib")	// these libs to your linker input
#endif

namespace olc
{
	class Platform_Windows : public olc::Platform
	{
	private:
		HWND olc_hWnd = nullptr;
		std::wstring wsAppName;

		std::wstring ConvertS2W(std::string s)
		{
#ifdef __MINGW32__
			wchar_t* buffer = new wchar_t[s.length() + 1];
			mbstowcs(buffer, s.c_str(), s.length());
			buffer[s.length()] = L'\0';
#else
			int count = MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, NULL, 0);
			wchar_t* buffer = new wchar_t[count];
			MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, buffer, count);
#endif
			std::wstring w(buffer);
			delete[] buffer;
			return w;
		}



	public:
		virtual olc::rcode ApplicationStartUp() override { return olc::rcode::OK; }
		virtual olc::rcode ApplicationCleanUp() override { return olc::rcode::OK; }
		virtual olc::rcode ThreadStartUp() override { return olc::rcode::OK; }

		virtual olc::rcode ThreadCleanUp() override
		{
			renderer->DestroyDevice();
			PostMessage(olc_hWnd, WM_DESTROY, 0, 0);
			return olc::OK;
		}

		virtual olc::rcode CreateGraphics(bool bFullScreen, bool bEnableVSYNC, const olc::vi2d& vViewPos, const olc::vi2d& vViewSize) override
		{
			if (renderer->CreateDevice({ olc_hWnd }, bFullScreen, bEnableVSYNC) == olc::rcode::OK)
			{
				renderer->UpdateViewport(vViewPos, vViewSize);
				return olc::rcode::OK;
			}
			else
				return olc::rcode::FAIL;
		}

		virtual olc::rcode CreateWindowPane(const olc::vi2d& vWindowPos, olc::vi2d& vWindowSize, bool bFullScreen) override
		{
			WNDCLASS wc;
			wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
			wc.hCursor = LoadCursor(NULL, IDC_ARROW);
			wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
			wc.hInstance = GetModuleHandle(nullptr);
			wc.lpfnWndProc = olc_WindowEvent;
			wc.cbClsExtra = 0;
			wc.cbWndExtra = 0;
			wc.lpszMenuName = nullptr;
			wc.hbrBackground = nullptr;
			wc.lpszClassName = olcT("OLC_PIXEL_GAME_ENGINE");
			RegisterClass(&wc);

			// Define window furniture
			DWORD dwExStyle = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
			DWORD dwStyle = WS_CAPTION | WS_SYSMENU | WS_VISIBLE | WS_THICKFRAME;

			olc::vi2d vTopLeft = vWindowPos;

			// Handle Fullscreen
			if (bFullScreen)
			{
				dwExStyle = 0;
				dwStyle = WS_VISIBLE | WS_POPUP;
				HMONITOR hmon = MonitorFromWindow(olc_hWnd, MONITOR_DEFAULTTONEAREST);
				MONITORINFO mi = { sizeof(mi) };
				if (!GetMonitorInfo(hmon, &mi)) return olc::rcode::FAIL;
				vWindowSize = { mi.rcMonitor.right, mi.rcMonitor.bottom };
				vTopLeft.x = 0;
				vTopLeft.y = 0;
			}

			// Keep client size as requested
			RECT rWndRect = { 0, 0, vWindowSize.x, vWindowSize.y };
			AdjustWindowRectEx(&rWndRect, dwStyle, FALSE, dwExStyle);
			int width = rWndRect.right - rWndRect.left;
			int height = rWndRect.bottom - rWndRect.top;

			olc_hWnd = CreateWindowEx(dwExStyle, olcT("OLC_PIXEL_GAME_ENGINE"), olcT(""), dwStyle,
				vTopLeft.x, vTopLeft.y, width, height, NULL, NULL, GetModuleHandle(nullptr), this);

			DragAcceptFiles(olc_hWnd, true);

			// Create Keyboard Mapping
			mapKeys[0x00] = Key::NONE;
			mapKeys[0x41] = Key::A; mapKeys[0x42] = Key::B; mapKeys[0x43] = Key::C; mapKeys[0x44] = Key::D; mapKeys[0x45] = Key::E;
			mapKeys[0x46] = Key::F; mapKeys[0x47] = Key::G; mapKeys[0x48] = Key::H; mapKeys[0x49] = Key::I; mapKeys[0x4A] = Key::J;
			mapKeys[0x4B] = Key::K; mapKeys[0x4C] = Key::L; mapKeys[0x4D] = Key::M; mapKeys[0x4E] = Key::N; mapKeys[0x4F] = Key::O;
			mapKeys[0x50] = Key::P; mapKeys[0x51] = Key::Q; mapKeys[0x52] = Key::R; mapKeys[0x53] = Key::S; mapKeys[0x54] = Key::T;
			mapKeys[0x55] = Key::U; mapKeys[0x56] = Key::V; mapKeys[0x57] = Key::W; mapKeys[0x58] = Key::X; mapKeys[0x59] = Key::Y;
			mapKeys[0x5A] = Key::Z;

			mapKeys[VK_F1] = Key::F1; mapKeys[VK_F2] = Key::F2; mapKeys[VK_F3] = Key::F3; mapKeys[VK_F4] = Key::F4;
			mapKeys[VK_F5] = Key::F5; mapKeys[VK_F6] = Key::F6; mapKeys[VK_F7] = Key::F7; mapKeys[VK_F8] = Key::F8;
			mapKeys[VK_F9] = Key::F9; mapKeys[VK_F10] = Key::F10; mapKeys[VK_F11] = Key::F11; mapKeys[VK_F12] = Key::F12;

			mapKeys[VK_DOWN] = Key::DOWN; mapKeys[VK_LEFT] = Key::LEFT; mapKeys[VK_RIGHT] = Key::RIGHT; mapKeys[VK_UP] = Key::UP;
			//mapKeys[VK_RETURN] = Key::ENTER;// mapKeys[VK_RETURN] = Key::RETURN;
			
			mapKeys[VK_BACK] = Key::BACK; mapKeys[VK_ESCAPE] = Key::ESCAPE; mapKeys[VK_RETURN] = Key::ENTER; mapKeys[VK_PAUSE] = Key::PAUSE;
			mapKeys[VK_SCROLL] = Key::SCROLL; mapKeys[VK_TAB] = Key::TAB; mapKeys[VK_DELETE] = Key::DEL; mapKeys[VK_HOME] = Key::HOME;
			mapKeys[VK_END] = Key::END; mapKeys[VK_PRIOR] = Key::PGUP; mapKeys[VK_NEXT] = Key::PGDN; mapKeys[VK_INSERT] = Key::INS;
			mapKeys[VK_SHIFT] = Key::SHIFT; mapKeys[VK_CONTROL] = Key::CTRL;
			mapKeys[VK_SPACE] = Key::SPACE;

			mapKeys[0x30] = Key::K0; mapKeys[0x31] = Key::K1; mapKeys[0x32] = Key::K2; mapKeys[0x33] = Key::K3; mapKeys[0x34] = Key::K4;
			mapKeys[0x35] = Key::K5; mapKeys[0x36] = Key::K6; mapKeys[0x37] = Key::K7; mapKeys[0x38] = Key::K8; mapKeys[0x39] = Key::K9;

			mapKeys[VK_NUMPAD0] = Key::NP0; mapKeys[VK_NUMPAD1] = Key::NP1; mapKeys[VK_NUMPAD2] = Key::NP2; mapKeys[VK_NUMPAD3] = Key::NP3; mapKeys[VK_NUMPAD4] = Key::NP4;
			mapKeys[VK_NUMPAD5] = Key::NP5; mapKeys[VK_NUMPAD6] = Key::NP6; mapKeys[VK_NUMPAD7] = Key::NP7; mapKeys[VK_NUMPAD8] = Key::NP8; mapKeys[VK_NUMPAD9] = Key::NP9;
			mapKeys[VK_MULTIPLY] = Key::NP_MUL; mapKeys[VK_ADD] = Key::NP_ADD; mapKeys[VK_DIVIDE] = Key::NP_DIV; mapKeys[VK_SUBTRACT] = Key::NP_SUB; mapKeys[VK_DECIMAL] = Key::NP_DECIMAL;

			// Thanks scripticuk
			mapKeys[VK_OEM_1] = Key::OEM_1;			// On US and UK keyboards this is the ';:' key
			mapKeys[VK_OEM_2] = Key::OEM_2;			// On US and UK keyboards this is the '/?' key
			mapKeys[VK_OEM_3] = Key::OEM_3;			// On US keyboard this is the '~' key
			mapKeys[VK_OEM_4] = Key::OEM_4;			// On US and UK keyboards this is the '[{' key
			mapKeys[VK_OEM_5] = Key::OEM_5;			// On US keyboard this is '\|' key.
			mapKeys[VK_OEM_6] = Key::OEM_6;			// On US and UK keyboards this is the ']}' key
			mapKeys[VK_OEM_7] = Key::OEM_7;			// On US keyboard this is the single/double quote key. On UK, this is the single quote/@ symbol key
			mapKeys[VK_OEM_8] = Key::OEM_8;			// miscellaneous characters. Varies by keyboard
			mapKeys[VK_OEM_PLUS] = Key::EQUALS;		// the '+' key on any keyboard
			mapKeys[VK_OEM_COMMA] = Key::COMMA;		// the comma key on any keyboard
			mapKeys[VK_OEM_MINUS] = Key::MINUS;		// the minus key on any keyboard
			mapKeys[VK_OEM_PERIOD] = Key::PERIOD;	// the period key on any keyboard
			mapKeys[VK_CAPITAL] = Key::CAPS_LOCK;
			return olc::OK;
		}

		virtual olc::rcode SetWindowTitle(const std::string& s) override
		{
#ifdef UNICODE
			SetWindowText(olc_hWnd, ConvertS2W(s).c_str());
#else
			SetWindowText(olc_hWnd, s.c_str());
#endif
			return olc::OK;
		}

		virtual olc::rcode StartSystemEventLoop() override
		{
			MSG msg;
			while (GetMessage(&msg, NULL, 0, 0) > 0)
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
			return olc::OK;
		}

		virtual olc::rcode HandleSystemEvent() override { return olc::rcode::FAIL; }

		// Windows Event Handler - this is statically connected to the windows event system
		static LRESULT CALLBACK olc_WindowEvent(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
		{
			switch (uMsg)
			{
			case WM_MOUSEMOVE:
			{
				// Thanks @ForAbby (Discord)
				uint16_t x = lParam & 0xFFFF; uint16_t y = (lParam >> 16) & 0xFFFF;
				int16_t ix = *(int16_t*)&x;   int16_t iy = *(int16_t*)&y;
				ptrPGE->olc_UpdateMouse(ix, iy);
				return 0;
			}
			case WM_SIZE:       ptrPGE->olc_UpdateWindowSize(lParam & 0xFFFF, (lParam >> 16) & 0xFFFF);	return 0;
			case WM_MOUSEWHEEL:	ptrPGE->olc_UpdateMouseWheel(GET_WHEEL_DELTA_WPARAM(wParam));           return 0;
			case WM_MOUSELEAVE: ptrPGE->olc_UpdateMouseFocus(false);                                    return 0;
			case WM_SETFOCUS:	ptrPGE->olc_UpdateKeyFocus(true);                                       return 0;
			case WM_KILLFOCUS:	ptrPGE->olc_UpdateKeyFocus(false);                                      return 0;
			case WM_KEYDOWN:	ptrPGE->olc_UpdateKeyState(mapKeys[wParam], true);                      return 0;
			case WM_KEYUP:		ptrPGE->olc_UpdateKeyState(mapKeys[wParam], false);                     return 0;
			case WM_SYSKEYDOWN: ptrPGE->olc_UpdateKeyState(mapKeys[wParam], true);						return 0;
			case WM_SYSKEYUP:	ptrPGE->olc_UpdateKeyState(mapKeys[wParam], false);						return 0;
			case WM_LBUTTONDOWN:ptrPGE->olc_UpdateMouseState(0, true);                                  return 0;
			case WM_LBUTTONUP:	ptrPGE->olc_UpdateMouseState(0, false);                                 return 0;
			case WM_RBUTTONDOWN:ptrPGE->olc_UpdateMouseState(1, true);                                  return 0;
			case WM_RBUTTONUP:	ptrPGE->olc_UpdateMouseState(1, false);                                 return 0;
			case WM_MBUTTONDOWN:ptrPGE->olc_UpdateMouseState(2, true);                                  return 0;
			case WM_MBUTTONUP:	ptrPGE->olc_UpdateMouseState(2, false);                                 return 0;
			case WM_DROPFILES:
			{
				// This is all eww...
				HDROP drop = (HDROP)wParam;
				
				uint32_t nFiles = DragQueryFile(drop, 0xFFFFFFFF, nullptr, 0);
				std::vector<std::string> vFiles;
				for (uint32_t i = 0; i < nFiles; i++)
				{
					TCHAR dfbuffer[256]{};
					uint32_t len = DragQueryFile(drop, i, nullptr, 0);
					DragQueryFile(drop, i, dfbuffer, 256);
#ifdef UNICODE
	#ifdef __MINGW32__
					char* buffer = new char[len + 1];
					wcstombs(buffer, dfbuffer, len);
					buffer[len] = '\0';
	#else
					int count = WideCharToMultiByte(CP_UTF8, 0, dfbuffer, -1, NULL, 0, NULL, NULL);
					char* buffer = new char[count];
					WideCharToMultiByte(CP_UTF8, 0, dfbuffer, -1, buffer, count, NULL, NULL);
	#endif				
					vFiles.push_back(std::string(buffer));
					delete[] buffer;
#else
					vFiles.push_back(std::string(dfbuffer));
#endif
				}

				// Even more eww...
				POINT p; DragQueryPoint(drop, &p);
				ptrPGE->olc_DropFiles(p.x, p.y, vFiles);
				DragFinish(drop);
				return 0;
			} 
			break;


			case WM_CLOSE:		ptrPGE->olc_Terminate();                                                return 0;
			case WM_DESTROY:	PostQuitMessage(0); DestroyWindow(hWnd);								return 0;
			}
			return DefWindowProc(hWnd, uMsg, wParam, lParam);
		}
	};
}
#endif
// O------------------------------------------------------------------------------O
// | END PLATFORM: MICROSOFT WINDOWS XP, VISTA, 7, 8, 10                          |
// O------------------------------------------------------------------------------O
#pragma endregion 

#pragma region platform_linux
// O------------------------------------------------------------------------------O
// | START PLATFORM: LINUX                                                        |
// O------------------------------------------------------------------------------O
#if defined(OLC_PLATFORM_X11)
namespace olc
{
	class Platform_Linux : public olc::Platform
	{
	private:
		X11::Display* olc_Display = nullptr;
		X11::Window					 olc_WindowRoot;
		X11::Window					 olc_Window;
		X11::XVisualInfo* olc_VisualInfo;
		X11::Colormap                olc_ColourMap;
		X11::XSetWindowAttributes    olc_SetWindowAttribs;

	public:
		virtual olc::rcode ApplicationStartUp() override
		{
			return olc::rcode::OK;
		}

		virtual olc::rcode ApplicationCleanUp() override
		{
			XDestroyWindow(olc_Display, olc_Window);
			return olc::rcode::OK;
		}

		virtual olc::rcode ThreadStartUp() override
		{
			return olc::rcode::OK;
		}

		virtual olc::rcode ThreadCleanUp() override
		{
			renderer->DestroyDevice();
			return olc::OK;
		}

		virtual olc::rcode CreateGraphics(bool bFullScreen, bool bEnableVSYNC, const olc::vi2d& vViewPos, const olc::vi2d& vViewSize) override
		{
			if (renderer->CreateDevice({ olc_Display, &olc_Window, olc_VisualInfo }, bFullScreen, bEnableVSYNC) == olc::rcode::OK)
			{
				renderer->UpdateViewport(vViewPos, vViewSize);
				return olc::rcode::OK;
			}
			else
				return olc::rcode::FAIL;
		}

		virtual olc::rcode CreateWindowPane(const olc::vi2d& vWindowPos, olc::vi2d& vWindowSize, bool bFullScreen) override
		{
			using namespace X11;
			XInitThreads();

			// Grab the deafult display and window
			olc_Display = XOpenDisplay(NULL);
			olc_WindowRoot = DefaultRootWindow(olc_Display);

			// Based on the display capabilities, configure the appearance of the window
			GLint olc_GLAttribs[] = { GLX_RGBA, GLX_DEPTH_SIZE, 24, GLX_DOUBLEBUFFER, None };
			olc_VisualInfo = glXChooseVisual(olc_Display, 0, olc_GLAttribs);
			olc_ColourMap = XCreateColormap(olc_Display, olc_WindowRoot, olc_VisualInfo->visual, AllocNone);
			olc_SetWindowAttribs.colormap = olc_ColourMap;

			// Register which events we are interested in receiving
			olc_SetWindowAttribs.event_mask = ExposureMask | KeyPressMask | KeyReleaseMask |
				ButtonPressMask | ButtonReleaseMask | PointerMotionMask | FocusChangeMask | StructureNotifyMask;

			// Create the window
			olc_Window = XCreateWindow(olc_Display, olc_WindowRoot, vWindowPos.x, vWindowPos.y,
				vWindowSize.x, vWindowSize.y,
				0, olc_VisualInfo->depth, InputOutput, olc_VisualInfo->visual,
				CWColormap | CWEventMask, &olc_SetWindowAttribs);

			Atom wmDelete = XInternAtom(olc_Display, "WM_DELETE_WINDOW", true);
			XSetWMProtocols(olc_Display, olc_Window, &wmDelete, 1);

			XMapWindow(olc_Display, olc_Window);
			XStoreName(olc_Display, olc_Window, "OneLoneCoder.com - Pixel Game Engine");

			if (bFullScreen) // Thanks DragonEye, again :D
			{
				Atom wm_state;
				Atom fullscreen;
				wm_state = XInternAtom(olc_Display, "_NET_WM_STATE", False);
				fullscreen = XInternAtom(olc_Display, "_NET_WM_STATE_FULLSCREEN", False);
				XEvent xev{ 0 };
				xev.type = ClientMessage;
				xev.xclient.window = olc_Window;
				xev.xclient.message_type = wm_state;
				xev.xclient.format = 32;
				xev.xclient.data.l[0] = (bFullScreen ? 1 : 0);   // the action (0: off, 1: on, 2: toggle)
				xev.xclient.data.l[1] = fullscreen;             // first property to alter
				xev.xclient.data.l[2] = 0;                      // second property to alter
				xev.xclient.data.l[3] = 0;                      // source indication
				XMapWindow(olc_Display, olc_Window);
				XSendEvent(olc_Display, DefaultRootWindow(olc_Display), False,
					SubstructureRedirectMask | SubstructureNotifyMask, &xev);
				XFlush(olc_Display);
				XWindowAttributes gwa;
				XGetWindowAttributes(olc_Display, olc_Window, &gwa);
				vWindowSize.x = gwa.width;
				vWindowSize.y = gwa.height;
			}

			// Create Keyboard Mapping
			mapKeys[0x00] = Key::NONE;
			mapKeys[0x61] = Key::A; mapKeys[0x62] = Key::B; mapKeys[0x63] = Key::C; mapKeys[0x64] = Key::D; mapKeys[0x65] = Key::E;
			mapKeys[0x66] = Key::F; mapKeys[0x67] = Key::G; mapKeys[0x68] = Key::H; mapKeys[0x69] = Key::I; mapKeys[0x6A] = Key::J;
			mapKeys[0x6B] = Key::K; mapKeys[0x6C] = Key::L; mapKeys[0x6D] = Key::M; mapKeys[0x6E] = Key::N; mapKeys[0x6F] = Key::O;
			mapKeys[0x70] = Key::P; mapKeys[0x71] = Key::Q; mapKeys[0x72] = Key::R; mapKeys[0x73] = Key::S; mapKeys[0x74] = Key::T;
			mapKeys[0x75] = Key::U; mapKeys[0x76] = Key::V; mapKeys[0x77] = Key::W; mapKeys[0x78] = Key::X; mapKeys[0x79] = Key::Y;
			mapKeys[0x7A] = Key::Z;

			mapKeys[XK_F1] = Key::F1; mapKeys[XK_F2] = Key::F2; mapKeys[XK_F3] = Key::F3; mapKeys[XK_F4] = Key::F4;
			mapKeys[XK_F5] = Key::F5; mapKeys[XK_F6] = Key::F6; mapKeys[XK_F7] = Key::F7; mapKeys[XK_F8] = Key::F8;
			mapKeys[XK_F9] = Key::F9; mapKeys[XK_F10] = Key::F10; mapKeys[XK_F11] = Key::F11; mapKeys[XK_F12] = Key::F12;

			mapKeys[XK_Down] = Key::DOWN; mapKeys[XK_Left] = Key::LEFT; mapKeys[XK_Right] = Key::RIGHT; mapKeys[XK_Up] = Key::UP;
			mapKeys[XK_KP_Enter] = Key::ENTER; mapKeys[XK_Return] = Key::ENTER;

			mapKeys[XK_BackSpace] = Key::BACK; mapKeys[XK_Escape] = Key::ESCAPE; mapKeys[XK_Linefeed] = Key::ENTER;	mapKeys[XK_Pause] = Key::PAUSE;
			mapKeys[XK_Scroll_Lock] = Key::SCROLL; mapKeys[XK_Tab] = Key::TAB; mapKeys[XK_Delete] = Key::DEL; mapKeys[XK_Home] = Key::HOME;
			mapKeys[XK_End] = Key::END; mapKeys[XK_Page_Up] = Key::PGUP; mapKeys[XK_Page_Down] = Key::PGDN;	mapKeys[XK_Insert] = Key::INS;
			mapKeys[XK_Shift_L] = Key::SHIFT; mapKeys[XK_Shift_R] = Key::SHIFT; mapKeys[XK_Control_L] = Key::CTRL; mapKeys[XK_Control_R] = Key::CTRL;
			mapKeys[XK_space] = Key::SPACE; mapKeys[XK_period] = Key::PERIOD;

			mapKeys[XK_0] = Key::K0; mapKeys[XK_1] = Key::K1; mapKeys[XK_2] = Key::K2; mapKeys[XK_3] = Key::K3; mapKeys[XK_4] = Key::K4;
			mapKeys[XK_5] = Key::K5; mapKeys[XK_6] = Key::K6; mapKeys[XK_7] = Key::K7; mapKeys[XK_8] = Key::K8; mapKeys[XK_9] = Key::K9;

			mapKeys[XK_KP_0] = Key::NP0; mapKeys[XK_KP_1] = Key::NP1; mapKeys[XK_KP_2] = Key::NP2; mapKeys[XK_KP_3] = Key::NP3; mapKeys[XK_KP_4] = Key::NP4;
			mapKeys[XK_KP_5] = Key::NP5; mapKeys[XK_KP_6] = Key::NP6; mapKeys[XK_KP_7] = Key::NP7; mapKeys[XK_KP_8] = Key::NP8; mapKeys[XK_KP_9] = Key::NP9;
			mapKeys[XK_KP_Multiply] = Key::NP_MUL; mapKeys[XK_KP_Add] = Key::NP_ADD; mapKeys[XK_KP_Divide] = Key::NP_DIV; mapKeys[XK_KP_Subtract] = Key::NP_SUB; mapKeys[XK_KP_Decimal] = Key::NP_DECIMAL;

			// These keys vary depending on the keyboard. I've included comments for US and UK keyboard layouts
			mapKeys[XK_semicolon] = Key::OEM_1;		// On US and UK keyboards this is the ';:' key
			mapKeys[XK_slash] = Key::OEM_2;			// On US and UK keyboards this is the '/?' key
			mapKeys[XK_asciitilde] = Key::OEM_3;	// On US keyboard this is the '~' key
			mapKeys[XK_bracketleft] = Key::OEM_4;	// On US and UK keyboards this is the '[{' key
			mapKeys[XK_backslash] = Key::OEM_5;		// On US keyboard this is '\|' key.
			mapKeys[XK_bracketright] = Key::OEM_6;	// On US and UK keyboards this is the ']}' key
			mapKeys[XK_apostrophe] = Key::OEM_7;	// On US keyboard this is the single/double quote key. On UK, this is the single quote/@ symbol key
			mapKeys[XK_numbersign] = Key::OEM_8;	// miscellaneous characters. Varies by keyboard. I believe this to be the '#~' key on UK keyboards
			mapKeys[XK_equal] = Key::EQUALS;		// the '+' key on any keyboard
			mapKeys[XK_comma] = Key::COMMA;			// the comma key on any keyboard
			mapKeys[XK_minus] = Key::MINUS;			// the minus key on any keyboard			

			mapKeys[XK_Caps_Lock] = Key::CAPS_LOCK;

			return olc::OK;
		}

		virtual olc::rcode SetWindowTitle(const std::string& s) override
		{
			X11::XStoreName(olc_Display, olc_Window, s.c_str());
			return olc::OK;
		}

		virtual olc::rcode StartSystemEventLoop() override
		{
			return olc::OK;
		}

		virtual olc::rcode HandleSystemEvent() override
		{
			using namespace X11;
			// Handle Xlib Message Loop - we do this in the
			// same thread that OpenGL was created so we dont
			// need to worry too much about multithreading with X11
			XEvent xev;
			while (XPending(olc_Display))
			{
				XNextEvent(olc_Display, &xev);
				if (xev.type == Expose)
				{
					XWindowAttributes gwa;
					XGetWindowAttributes(olc_Display, olc_Window, &gwa);
					ptrPGE->olc_UpdateWindowSize(gwa.width, gwa.height);
				}
				else if (xev.type == ConfigureNotify)
				{
					XConfigureEvent xce = xev.xconfigure;
					ptrPGE->olc_UpdateWindowSize(xce.width, xce.height);
				}
				else if (xev.type == KeyPress)
				{
					KeySym sym = XLookupKeysym(&xev.xkey, 0);
					ptrPGE->olc_UpdateKeyState(mapKeys[sym], true);
					XKeyEvent* e = (XKeyEvent*)&xev; // Because DragonEye loves numpads
					XLookupString(e, NULL, 0, &sym, NULL);
					ptrPGE->olc_UpdateKeyState(mapKeys[sym], true);
				}
				else if (xev.type == KeyRelease)
				{
					KeySym sym = XLookupKeysym(&xev.xkey, 0);
					ptrPGE->olc_UpdateKeyState(mapKeys[sym], false);
					XKeyEvent* e = (XKeyEvent*)&xev;
					XLookupString(e, NULL, 0, &sym, NULL);
					ptrPGE->olc_UpdateKeyState(mapKeys[sym], false);
				}
				else if (xev.type == ButtonPress)
				{
					switch (xev.xbutton.button)
					{
					case 1:	ptrPGE->olc_UpdateMouseState(0, true); break;
					case 2:	ptrPGE->olc_UpdateMouseState(2, true); break;
					case 3:	ptrPGE->olc_UpdateMouseState(1, true); break;
					case 4:	ptrPGE->olc_UpdateMouseWheel(120); break;
					case 5:	ptrPGE->olc_UpdateMouseWheel(-120); break;
					default: break;
					}
				}
				else if (xev.type == ButtonRelease)
				{
					switch (xev.xbutton.button)
					{
					case 1:	ptrPGE->olc_UpdateMouseState(0, false); break;
					case 2:	ptrPGE->olc_UpdateMouseState(2, false); break;
					case 3:	ptrPGE->olc_UpdateMouseState(1, false); break;
					default: break;
					}
				}
				else if (xev.type == MotionNotify)
				{
					ptrPGE->olc_UpdateMouse(xev.xmotion.x, xev.xmotion.y);
				}
				else if (xev.type == FocusIn)
				{
					ptrPGE->olc_UpdateKeyFocus(true);
				}
				else if (xev.type == FocusOut)
				{
					ptrPGE->olc_UpdateKeyFocus(false);
				}
				else if (xev.type == ClientMessage)
				{
					ptrPGE->olc_Terminate();
				}
			}
			return olc::OK;
		}
	};
}
#endif
// O------------------------------------------------------------------------------O
// | END PLATFORM: LINUX                                                          |
// O------------------------------------------------------------------------------O
#pragma endregion



#endif // Headless

// O------------------------------------------------------------------------------O
// | olcPixelGameEngine Auto-Configuration                                        |
// O------------------------------------------------------------------------------O


//#endif // End OLC_PGE_APPLICATION

// O------------------------------------------------------------------------------O
// | END OF OLC_PGE_APPLICATION                                                   |
// O------------------------------------------------------------------------------O
