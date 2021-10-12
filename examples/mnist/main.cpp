#include "header.hpp"
#include "sdl_surface.hpp"
#include "sdl_texture.hpp"

#include <execution>
#include <fmt/core.h>
#include <string>
#include <thread>

struct TrainAsset {
    Uint8* data;
    SDL_Texture* texture;
};

struct TextSize {
    int width;
    int height;
};

#ifdef __linux

int parseLine(char* line) {
    // This assumes that a digit will be found and the line ends in " Kb".
    int i         = (int)strlen(line);
    const char* p = line;
    while (*p < '0' || *p > '9') p++;
    line[i - 3] = '\0';
    i           = std::atoi(p);
    return i;
}

int current_used_ram() {
    FILE* file = fopen("/proc/self/status", "r");
    int result = -1;
    char line[128];

    while (fgets(line, 128, file) != NULL) {
        if (strncmp(line, "VmRSS:", 6) == 0) {
            result = parseLine(line);
            break;
        }
    }
    fclose(file);
    return result;
}

#endif

static void render_user_input(SDL_Event* const event, Uint32* user_input_pixels) {
    const std::int32_t& mouseX = event->motion.x - 200;
    const std::int32_t& mouseY = event->motion.y - 20;
    const auto& is_mouse_valid = [=] {
        const auto& is_valid = [](std::int32_t cord, std::int32_t max_size = 200) {
            return cord >= 0 && cord <= max_size;
        };
        return is_valid(mouseX) && is_valid(mouseY);
    };
    if (is_mouse_valid()) {
        const auto& color                                  = 255;
        user_input_pixels[(mouseY - 2) * 200 + mouseX - 1] = color;
        user_input_pixels[(mouseY - 2) * 200 + mouseX]     = color;
        user_input_pixels[(mouseY - 1) * 200 + mouseX - 1] = color;
        user_input_pixels[(mouseY - 1) * 200 + mouseX]     = color;
        user_input_pixels[mouseY * 200 + mouseX - 1]       = color;
        user_input_pixels[mouseY * 200 + mouseX]           = color;
        user_input_pixels[mouseY * 200 + mouseX + 1]       = color;
        user_input_pixels[(mouseY + 1) * 200 + mouseX]     = color;
        user_input_pixels[(mouseY + 1) * 200 + mouseX + 1] = color;
        user_input_pixels[(mouseY + 2) * 200 + mouseX]     = color;
        user_input_pixels[(mouseY + 2) * 200 + mouseX + 1] = color;
    }
}

static void testing_stage() noexcept {
    std::array<double, nn_inputs_size> inputs{};
    std::for_each(std::execution::par, inputs.begin(), inputs.end(), [&](double& input) {
        std::lock_guard<std::mutex> guard(m);

        const auto& i      = std::distance(inputs.begin(), &input);
        const auto& bright = std::get<1>(mnist[label_data::test_images])[test_index][i];
        input              = static_cast<double>(bright) / 255;
    });
    const auto& label = std::get<0>(mnist[label_data::test_labels])[test_index];

    const auto& prediction = nn.predict(inputs.data());
    const auto& guess      = std::distance(std::max_element(prediction, prediction + nn_outputs_size), prediction + nn_outputs_size);
    delete[] prediction;

    total_tests++;
    if (guess == label) {
        total_correct++;
    }

    const auto& percent = 100 * (static_cast<float>(total_correct) / total_tests);
    percent_element     = fmt::format("Testing: {0:.2f}%", percent);
    //percent_ele.html(nf(percent, 2, 2) + '%');

    test_index++;
    if (test_index == std::get<0>(mnist[label_data::test_labels]).size()) {
        test_index = 0;
        std::clog << "finished test set\n";
        std::clog << percent << '\n';
        total_tests   = 0;
        total_correct = 0;
    }
}

static void guess_user_digit(Uint32* user_digit) {
    if (!user_has_drawing) {
        user_guess_element = fmt::format("User Guess: _");
        //user_guess_ele.html('_');
        return;
    }

    std::array<double, nn_inputs_size> inputs{};
    std::for_each(std::execution::par, inputs.begin(), inputs.end(), [&](double& input) {
        std::lock_guard<std::mutex> guard(m);

        const auto& i = std::distance(inputs.begin(), &input);
        input         = static_cast<double>(user_digit[i * 4]) / 255;
    });

    const auto& prediction  = nn.predict(inputs.data());
    const auto& max_element = std::max_element(prediction, prediction + nn_outputs_size);
    const auto& guess       = std::distance(prediction, max_element);
    delete[] prediction;

    user_guess_element = fmt::format("User Guess: {}", guess);
    //user_guess_ele.html(guess);
}

static void train(TrainAsset* train_asset, bool show) {
    std::array<double, nn_inputs_size> inputs{};
    std::for_each(std::execution::par, inputs.begin(), inputs.end(), [&](double& input) {
        std::lock_guard<std::mutex> guard(m);

        const auto& i      = std::distance(inputs.begin(), &input);
        const auto& bright = std::get<1>(mnist[label_data::train_images])[train_index][i];
        input              = static_cast<double>(bright) / 255;
        if (show) {
            const auto& index            = i % nn_inputs_size;
            train_asset->data[index + 0] = bright;
            train_asset->data[index + 1] = bright;
            train_asset->data[index + 2] = bright;
            train_asset->data[index + 3] = 255;
        }
    });
    if (show) {
        SDL_UpdateTexture(train_asset->texture, nullptr, train_asset->data, 28 * sizeof(Uint8));
    }

    // Do the neural network stuff;
    const auto& label = std::get<0>(mnist[label_data::train_labels])[train_index];
    //fmt::print(stderr, "INFO: {}\n", label);
    std::array<double, 10> targets{};
    targets[label] = 1;

    // fmt::print("{}\n", inputs);
    // fmt::print("{}\n", targets);

    // fmt::print("{}\n", train_index);

    //    const auto& prediction  = nn.predict(inputs.data());
    //    const auto& max_element = std::max_element(prediction, prediction + nn_outputs_size);
    //    const auto& guess       = std::distance(prediction, max_element);
    //    delete[] prediction;

    nn.train(inputs.data(), targets.data());
    train_index = (train_index + 1) % std::get<0>(mnist[label_data::train_labels]).size();
}

static void on_draw(Uint32* user_digit, TrainAsset* train_asset) {
    std::this_thread::sleep_for(std::chrono::milliseconds(2));
    guess_user_digit(user_digit);

    static constexpr std::uint32_t total1 = 5;
    for (std::uint32_t i = 0; i < total1; ++i) {
        if (i == (total1 - 1)) {
            train(train_asset, true);
        } else {
            train(train_asset, false);
        }
    }
    static constexpr std::uint32_t total2 = 25;
    for (std::uint32_t i = 0; i < total2; i++) {
        testing_stage();
    }
}

int main() {
    std::cout << "Done\n";

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) < 0) {
        std::cerr << "Error SDL2 Initialization : " << SDL_GetError();
        return -1;
    }

    if (TTF_Init() < 0) {
        std::cerr << "Error SDL2_ttf Initialization";
        return -1;
    }

    SDL_Window* window = SDL_CreateWindow("MNIST Program", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, widthI * 2, heightI * 3, SDL_WINDOW_OPENGL);
    if (window == nullptr) {
        std::cerr << "Error window creation\n"
                  << SDL_GetError() << '\n';
        return -1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == nullptr) {
        std::cerr << "Error renderer creation";
        return -1;
    }

    sdl::texture user_input_texture(renderer);
    user_input_texture.create(SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, 200, 200);
    auto* user_input_pixels = new Uint32[200 * 200];
    memset(user_input_pixels, 0, 200 * 200 * sizeof(Uint32));

    sdl::texture train_image_texture(renderer);
    train_image_texture.create(SDL_PIXELFORMAT_RGB332, SDL_TEXTUREACCESS_STREAMING, 28, 28);
    auto* train_image_pixels = new Uint8[28 * 28];
    memset(train_image_pixels, 0, 28 * 28 * sizeof(Uint8));

    TrainAsset train_asset{train_image_pixels, train_image_texture.get()};

    static TTF_Font* font = TTF_OpenFont("Hack-Regular.ttf", 18);
    if (font == nullptr) {
        std::cerr << "Error font opening\n"
                  << SDL_GetError() << '\n';
        return -1;
    }
    sdl::texture percent_texture(renderer, font);
    sdl::texture guess_texture(renderer, font);
    TextSize percent_text_size{};
    TextSize guess_text_size{};

    //std::thread poll(on_draw, user_input_pixels, &train_asset);

    bool leftMouseButtonDown = false;
    bool running             = true;
    while (running) {
        SDL_UpdateTexture(user_input_texture.get(), nullptr, user_input_pixels, 200 * sizeof(Uint32));
        percent_texture.loadFromText(percent_element.c_str(), {});
        guess_texture.loadFromText(user_guess_element.c_str(), {});
        SDL_QueryTexture(percent_texture.get(), nullptr, nullptr, &percent_text_size.width, &percent_text_size.height);
        SDL_QueryTexture(guess_texture.get(), nullptr, nullptr, &guess_text_size.width, &guess_text_size.height);

        SDL_Event e;
        if (SDL_PollEvent(&e)) {
            switch (e.type) {
            case SDL_QUIT:
                running = false;
                break;
            case SDL_MOUSEBUTTONUP:
                if (e.button.button == SDL_BUTTON_LEFT) {
                    leftMouseButtonDown = false;
                }
                break;
            case SDL_MOUSEBUTTONDOWN:
                if (e.button.button == SDL_BUTTON_LEFT) {
                    leftMouseButtonDown = true;
                    user_has_drawing    = true;
                }
                break;
            case SDL_MOUSEMOTION:
                if (leftMouseButtonDown) {
                    render_user_input(&e, user_input_pixels);
                }
                break;
            case SDL_KEYDOWN:
                if (e.key.keysym.sym == SDLK_ESCAPE) {
                    running = false;
                } else if (e.key.keysym.sym == SDLK_SPACE) {
                    user_has_drawing = false;
                    memset(user_input_pixels, 0, 200 * 200 * sizeof(Uint32));
#ifdef __linux
                } else if (e.key.keysym.sym == SDLK_r) {
                    const auto& used_ram = current_used_ram();
                    if (used_ram != -1) {
                        std::cout << "INFO: Current use of RAM " << std::to_string(used_ram / 1024) << "MB" << '\n';
                    }
#endif
                }
                break;
            }

#ifndef NDEBUG
            PrintEvent(&e);
#endif
        }

        on_draw(user_input_pixels, &train_asset);

        SDL_SetRenderDrawColor(renderer, red, green, blue, SDL_ALPHA_OPAQUE);
        SDL_RenderClear(renderer);

        SDL_Rect percentrect{280, 370, percent_text_size.width, percent_text_size.height};
        percent_texture.render(nullptr, &percentrect);
        SDL_Rect guessrect{280, 400, guess_text_size.width, guess_text_size.height};
        guess_texture.render(nullptr, &guessrect);
        SDL_Rect uinprect{200, 20, 200, 200};
        user_input_texture.render(nullptr, &uinprect);
        SDL_Rect dstrect{400, 20, 200, 200};
        train_image_texture.render(nullptr, &dstrect);

        SDL_RenderPresent(renderer);
    }

    delete[] user_input_pixels;
    delete[] train_image_pixels;
    SDL_DestroyTexture(train_image_texture.get());
    TTF_CloseFont(font);
    TTF_Quit();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}
