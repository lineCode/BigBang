#include <cwchar>
#include <cstdarg>
#include <cstdlib>

#include "text.hpp"
#include "textlet.hpp"

using namespace WarGrey::SCADA;

using namespace Windows::UI;
using namespace Windows::UI::Text;
using namespace Microsoft::Graphics::Canvas;
using namespace Microsoft::Graphics::Canvas::Text;

Textlet::Textlet(const wchar_t *fmt, ...) {
    static const int DEFAULT_POOL_SIZE = 1024;
    static wchar_t wpool[DEFAULT_POOL_SIZE];

    int bigSize = DEFAULT_POOL_SIZE - 1;
    wchar_t *pool;
    va_list argl;

    do {
        pool = (bigSize < DEFAULT_POOL_SIZE) ? wpool : (new wchar_t[bigSize + 1]);
        va_start(argl, fmt);
        int status = vswprintf(pool, bigSize + 1, fmt, argl);
        va_end(argl);
        if (status == -1) {
            bigSize = bigSize * 2 + 1;
            if (pool != wpool) delete[] pool;
            pool = nullptr;
        }
    } while (pool == nullptr);

    this->change_text(ref new Platform::String(pool));
    if (pool != wpool) delete[] pool;
}

Textlet::Textlet(Platform::String^ content) {
    this->change_text(content);
}

Textlet::~Textlet() {}

void Textlet::change_text(Platform::String^ content) {
    this->content = content;
    if (this->label_font == nullptr) {
        this->label_font = make_text_format();
    }
}

void Textlet::fill_extent(float x, float y, float* w, float* h) {
    TextExtent ts = get_text_extent(content, label_font);

    SET_VALUES(w, ts.width, h, ts.height);
};

void Textlet::draw(CanvasDrawingSession^ ds, float x, float y, float Width, float Height) {
    ds->DrawText(content, x, y, Colors::Snow, label_font);
}
