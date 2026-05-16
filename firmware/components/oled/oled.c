#include "include/oled.h"
#include "include/font8x8_basic.h"
#include "i2c.h"

// Inicjalizacja struktury i wyczyszczenie bufora
void ssd1306_init(SSD1306_t * dev, int width, int height)
{
    i2c_init(dev, width, height);

    // Inicjalizacja wewnetrznego bufora pamieci ekranu zerami
    for (int i = 0; i < dev->_pages; i++) 
    {
        memset(dev->_page[i]._segs, 0, 128);
    }
}

// Czyszczenie calego ekranu poprzez zapisanie go spacjami
void ssd1306_clear_screen(SSD1306_t * dev, bool invert)
{
    char space[16];
    memset(space, 0x00, sizeof(space));
    
    for (int page = 0; page < dev->_pages; page++) 
    {
        ssd1306_display_text(dev, page, space, sizeof(space), invert);
    }
}

// Zmiana kontrastu wyswietlacza
void ssd1306_contrast(SSD1306_t * dev, int contrast)
{
    i2c_contrast(dev, contrast);
}

// Rysowanie grafiki na ekranie oraz zapis do wewnetrznego bufora
void ssd1306_display_image(SSD1306_t * dev, int page, int seg, const uint8_t * images, int width)
{
    i2c_display_image(dev, page, seg, images, width);
	
    // Zapis przeslanej grafiki do wewnetrznego bufora ramki
    memcpy(&dev->_page[page]._segs[seg], images, width);
}

// Renderowanie tekstu przy uzyciu czcionki matrycowej 8x8
void ssd1306_display_text(SSD1306_t * dev, int page, const char * text, int text_len, bool invert)
{
    if (page >= dev->_pages) 
    {
        return;
    }
    
    int _text_len = text_len;
    if (_text_len > 16) 
    {
        _text_len = 16;
    }

    int seg = 0;
    uint8_t image[8];
    
    for (int i = 0; i < _text_len; i++) 
    {
        // Kopiowanie wzorca znaku z tablicy czcionek i wyslanie na ekran
        memcpy(image, font8x8_basic_tr[(uint8_t)text[i]], 8);
        ssd1306_display_image(dev, page, seg, image, 8);
        seg = seg + 8;
    }
}