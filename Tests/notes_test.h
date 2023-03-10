//
// Created by Tibor Vass on 05.03.23.
//

#ifndef AUDIO2MIDIPLUGIN_NOTES_TEST_H
#define AUDIO2MIDIPLUGIN_NOTES_TEST_H

#include <fstream>
#include <json.hpp>

#include "Notes.h"
#include "test_utils.h"
#include "Utils.h"

using json = nlohmann::json;

/*
 * Expected output was computed with basic-pitch's python implementation.
 *
 * To generate the expected note_events.output.json:
 *  - install python and basic-pitch
 *  - run:
 *      ( cd test_data && ../gen_note_events.py note_events.input.json notes.csv onsets.csv contours.csv note_events.output.json )
 */
bool notes_test() {
    // inputs are the model's output + JSON params
    std::ifstream f_notes_pg(std::string(TEST_DATA_DIR) + "/notes.csv");
    std::ifstream f_onsets_pg(std::string(TEST_DATA_DIR) + "/onsets.csv");
    std::ifstream f_contours_pg(std::string(TEST_DATA_DIR) + "/contours.csv");
    auto notes_pg_1d = test_utils::loadCSVDataFile<float>(f_notes_pg);
    auto onsets_pg_1d = test_utils::loadCSVDataFile<float>(f_onsets_pg);
    auto contours_pg_1d = test_utils::loadCSVDataFile<float>(f_contours_pg);

    std::ifstream f_input(std::string(TEST_DATA_DIR) + "/note_events.input.json");
    std::ifstream f_expected(std::string(TEST_DATA_DIR) + "/note_events.output.json");
    auto params = json::parse(f_input).get<Notes::ConvertParams>();
    auto expected = json::parse(f_expected).get<std::vector<Notes::Event>>();

    auto notes_pg = test_utils::convert_1d_to_2d<float>(notes_pg_1d, -1, NUM_FREQ_OUT);
    auto onsets_pg = test_utils::convert_1d_to_2d<float>(onsets_pg_1d, -1, NUM_FREQ_OUT);
    auto contours_pg = test_utils::convert_1d_to_2d<float>(contours_pg_1d, -1, NUM_FREQ_IN);

    Notes n;
    auto start_time = std::chrono::high_resolution_clock::now();
    auto note_events = n.convert(notes_pg, onsets_pg, contours_pg, params);
    auto stop_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration = stop_time - start_time;
    std::cout << "Done in " << duration.count() << " seconds" << std::endl;

    bool succeeded = true;
    if (note_events.size() != expected.size()) {
        std::cout << "FAIL: Got " << note_events.size() << " elements in array, expected " << expected.size() << std::endl;
        succeeded = false;
    }

    for(int i = 0; i < expected.size(); i++) {
        if (!(note_events[i] == expected[i])) {
            json res = note_events[i];
            json exp = expected[i];
            std::cout << "FAIL: Element " << i << " is:" << std::endl << "\t" << res << std::endl << "Expecting:" << std::endl << "\t" << exp << std::endl;
            succeeded = false;
        }
    }

    if (succeeded)
    {
        std::cout << "Success" << std::endl;
    }
    return succeeded;
}

#endif //AUDIO2MIDIPLUGIN_NOTES_TEST_H