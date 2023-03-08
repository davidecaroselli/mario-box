//#include <GLUT/glut.h>
//#include "core/SystemBus.h"
//#include "core/Cartridge.h"
//#include "core/NES.h"
//
//#include <string>
//
//GLuint glInitTexture(char* filename)
//{
//    GLuint t = 0;
//
//    glGenTextures( 1, &t );
//    glBindTexture(GL_TEXTURE_2D, t);
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
//    unsigned char data[] = {
//            255, 255, 0, 255,
//            255, 0, 0, 255,
//            0, 255, 0, 255,
//            0, 0, 255, 255,
//    };
//    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 2, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE, data );
//    return t;
//}
//
//void drawImage(GLuint file,
//               float x,
//               float y,
//               float w,
//               float h,
//               float angle)
//{
//    //glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
//    //glPushMatrix();
//    //glTranslatef(x, y, 0.0);
//    //glRotatef(angle, 0.0, 0.0, 1.0);
//
//    glBindTexture(GL_TEXTURE_2D, file);
//    glEnable(GL_TEXTURE_2D);
//
//    glBegin(GL_QUADS);
//    glTexCoord2f(0.0, 0.0); glVertex3f(x, y, 0.0f);
//    glTexCoord2f(0.0, 1.0); glVertex3f(x, y + h, 0.0f);
//    glTexCoord2f(1.0, 1.0); glVertex3f(x + w, y + h, 0.0f);
//    glTexCoord2f(1.0, 0.0); glVertex3f(x + w, y, 0.0f);
//    glEnd();
//
//    //glPopMatrix();
//}
//
//GLuint texture;
//void render()
//{
//    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
//    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
//
//    glEnable(GL_DEPTH_TEST);
//
//    glMatrixMode(GL_PROJECTION);
//    glLoadIdentity();
//    const double w = glutGet( GLUT_WINDOW_WIDTH );
//    const double h = glutGet( GLUT_WINDOW_HEIGHT );
//    gluPerspective(45.0, w / h, 0.1, 1000.0);
//
//    glMatrixMode(GL_MODELVIEW);
//    glLoadIdentity();
//    glTranslatef( 0, 0, -15 );
//
//    drawImage(texture, 0.0f, 0.0f, 10.0f, 10.0f, 0.0);
//
//    glutSwapBuffers();
//}
//
//#include <olcPixelGameEngine.h>
//
//int main(int argc, char* argv[])
//{
//    NES nes;

//
//    nes.print_cpu_registers();
//    for (int i = 0; i < 30; i++) {
//        nes.step();
//        nes.print_cpu_registers();
//    }
//
//
//    glutInit(&argc, argv);
//    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
//    glutInitWindowSize(600, 600);
//    glutCreateWindow("Applying Textures");
//    glutDisplayFunc(render);
//
//    texture = glInitTexture("/Users/davide/CLionProjects/nessy/image.png");
//
//    glutMainLoop();
//    return 0;
//}

#include "olcPixelGameEngine.h"
#include "core/NES.h"
#include "debug/ASM.h"
#include "platform/olc/olcCanvas.h"

class Example : public olc::PixelGameEngine {
public:
    olcCanvas screen;

    NES *nes;
    ASM *code = nullptr;

    Example(): screen(256, 240) {
        nes = new NES(&screen);
        sAppName = "Example";
    }

public:
    bool OnUserCreate() override {
        Cartridge *cartridge = Cartridge::load("/Users/davide/Desktop/nestest.nes");
//        Cartridge *cartridge = Cartridge::load("/Users/davide/Desktop/donkeykong.nes");
//        Cartridge *cartridge = Cartridge::load("/Users/davide/Desktop/supermariobros.nes");
        nes->insert(cartridge);
        code = ASM::decompile(&cartridge->prg);

        DrawUI();

        return true;
    }

    void DrawCode(int x, int y) {
        int i = 0;
        for (auto &line: code->snippet(nes->cpu.prg_counter, 8)) {
            DrawString(x, y + i * 10, line, i == 8 ? olc::WHITE : olc::GREY);
            i++;
        }
    }

    std::string hex(uint32_t n, uint8_t d) {
        std::string s(d, '0');
        for (int i = d - 1; i >= 0; i--, n >>= 4)
            s[i] = "0123456789ABCDEF"[n & 0xF];
        return s;
    };

    void DrawCPU(int x, int y) {
        DrawString(x, y, "STATUS:", olc::WHITE);
        DrawString(x + 64, y, "N", nes->cpu.get_status(C6502::Status::N) ? olc::GREEN : olc::RED);
        DrawString(x + 80, y, "V", nes->cpu.get_status(C6502::Status::V) ? olc::GREEN : olc::RED);
        DrawString(x + 96, y, "-", nes->cpu.get_status(C6502::Status::U) ? olc::GREEN : olc::RED);
        DrawString(x + 112, y, "B", nes->cpu.get_status(C6502::Status::B) ? olc::GREEN : olc::RED);
        DrawString(x + 128, y, "D", nes->cpu.get_status(C6502::Status::D) ? olc::GREEN : olc::RED);
        DrawString(x + 144, y, "I", nes->cpu.get_status(C6502::Status::I) ? olc::GREEN : olc::RED);
        DrawString(x + 160, y, "Z", nes->cpu.get_status(C6502::Status::Z) ? olc::GREEN : olc::RED);
        DrawString(x + 178, y, "C", nes->cpu.get_status(C6502::Status::C) ? olc::GREEN : olc::RED);
        DrawString(x, y + 10, "PC: $" + hex(nes->cpu.prg_counter, 4));
        DrawString(x, y + 20, "A: $" + hex(nes->cpu.a, 2) + "  [" + std::to_string(nes->cpu.a) + "]");
        DrawString(x, y + 30, "X: $" + hex(nes->cpu.x, 2) + "  [" + std::to_string(nes->cpu.x) + "]");
        DrawString(x, y + 40, "Y: $" + hex(nes->cpu.y, 2) + "  [" + std::to_string(nes->cpu.y) + "]");
        DrawString(x, y + 50, "Stack P: $" + hex(nes->cpu.stack_ptr, 4));
    }

    olcCanvas p0 = olcCanvas(4, 1);
    olcCanvas p1 = olcCanvas(4, 1);
    olcCanvas p2 = olcCanvas(4, 1);
    olcCanvas p3 = olcCanvas(4, 1);
    olcCanvas p4 = olcCanvas(4, 1);
    olcCanvas p5 = olcCanvas(4, 1);
    olcCanvas p6 = olcCanvas(4, 1);
    olcCanvas p7 = olcCanvas(4, 1);
    std::vector<olcCanvas *> palettes = {&p0, &p1, &p2, &p3, &p4, &p5, &p6, &p7};
    void DrawPalettes(int x, int y) {
        for (int p = 0; p < 8; p++) {
            nes->ppu.palettes.render(p, palettes[p]);
            DrawSprite(x + p * 33, y, &palettes[p]->sprite, 6);
        }
    }

    olcCanvas chr0 = olcCanvas(128, 128);
    olcCanvas chr1 = olcCanvas(128, 128);
    uint8_t palette_idx = 0;
    void DrawPatternTables(int x, int y) {
        Palette palette = nes->ppu.palettes.get(palette_idx);
        nes->cartridge->chr.render(0, palette, &chr0);
        nes->cartridge->chr.render(1, palette, &chr1);
        DrawSprite(x, y, &chr0.sprite);
        DrawSprite(x + 132, y, &chr1.sprite);
    }

    void DrawUI() {
        Clear(olc::DARK_BLUE);

        DrawCPU(530, 10);
        DrawCode(530, 80);
        DrawPalettes(530, 255);
        DrawPatternTables(530, 280);

        Palette palette = nes->ppu.palettes.get(palette_idx);

        for (uint8_t y = 0; y < 30; ++y) {
            for (uint8_t x = 0; x < 32; ++x) {
                uint8_t tile = nes->ppu.name_tables.at(0, x, y);
                nes->cartridge->chr.render_tile(0, tile, palette, &screen, x * 8, y * 8);
            }
        }

        DrawSprite(10, 10, &screen.sprite, 2);
    }

    bool bEmulationRun = false;
    float fResidualTime = 0;
    bool jump = true;

    bool OnUserUpdate(float fElapsedTime) override {
//        if (jump) {
//            while (true) {
//                nes->step();
//                if (nes->cpu.prg_counter == 0xC07E) break;
//                if (nes->cpu.prg_counter == 0xC306) break;
//            }
//            jump = false;
//            bEmulationRun = false;
//        }

        bool draw_ui = true;

        if (bEmulationRun) {
            if (fResidualTime > 0.0f) {
                fResidualTime -= fElapsedTime;
                draw_ui = false;
            } else {
                fResidualTime += (1.0f / 60.0f) - fElapsedTime;
                nes->frame();
            }
        } else {
            // Emulate code step-by-step
            if (GetKey(olc::Key::C).bPressed) nes->step();

            // Emulate one whole frame
            if (GetKey(olc::Key::F).bPressed) nes->frame();
        }

        if (GetKey(olc::Key::SPACE).bPressed) bEmulationRun = !bEmulationRun;
        if (GetKey(olc::Key::R).bPressed) nes->reset();
        if (GetKey(olc::Key::P).bPressed) palette_idx = (palette_idx + 1) & 0x07;

        if (draw_ui) DrawUI();

        return true;
    }
};


int main() {
    Example demo;
    int scale = 1;
    if (demo.Construct(786, 510, scale, scale))
        demo.Start();

    return 0;
}