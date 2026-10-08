#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <string>

#include <httplib.h>
#include <nlohmann/json.hpp>

#include "board.h"
#include "search.h"

using json = nlohmann::json;

namespace {

struct Difficulty {
    int depth;
    int timeMs;
};

Difficulty difficultyFor(const std::string& level) {
    if (level == "easy") return {1, 300};
    if (level == "medium") return {3, 1000};
    if (level == "hard") return {64, 2500};
    throw std::invalid_argument("Difficulty must be easy, medium, or hard");
}

void sendError(httplib::Response& res, int status, const std::string& message) {
    res.status = status;
    res.set_content(json{{"error", message}}.dump(), "application/json");
}

}  // namespace

int main() {
    httplib::Server server;

    server.set_default_headers({
        {"Access-Control-Allow-Origin", "*"},
        {"Access-Control-Allow-Methods", "GET, POST, OPTIONS"},
        {"Access-Control-Allow-Headers", "Content-Type"},
    });
    server.Options(".*", [](const httplib::Request&, httplib::Response& res) {
        res.status = 204;
    });

    server.Get("/health", [](const httplib::Request&, httplib::Response& res) {
        res.set_content(R"({"status":"ok"})", "application/json");
    });

    server.Post("/move", [](const httplib::Request& req, httplib::Response& res) {
        try {
            json body = json::parse(req.body);
            Board board(body.at("fen").get<std::string>());
            Difficulty difficulty = difficultyFor(body.value("difficulty", "medium"));

            SearchResult result = findBestMove(board, difficulty.depth, difficulty.timeMs);
            if (!result.hasMove) {
                return sendError(res, 400, "No legal moves: the game is over");
            }

            json response = {
                {"move", moveToUci(result.bestMove)},
                {"score", result.score},
                {"depth", result.depth},
                {"nodes", result.nodes},
                {"timeMs", result.timeMs},
            };
            res.set_content(response.dump(), "application/json");
        } catch (const json::exception& e) {
            sendError(res, 400, std::string("Invalid request: ") + e.what());
        } catch (const std::invalid_argument& e) {
            sendError(res, 400, e.what());
        }
    });

    const char* portEnv = std::getenv("PORT");
    int port = portEnv ? std::atoi(portEnv) : 8080;
    std::cout << "Chess engine API listening on port " << port << std::endl;
    server.listen("0.0.0.0", port);
    return 0;
}