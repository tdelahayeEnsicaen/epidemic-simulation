#include "Map.h"

#include <stdlib.h>
#include <stdio.h>

#include <ncurses.h>

FILE* pFile;
WINDOW* pLegendWindow;
WINDOW* pTilesWindow;
WINDOW* pCitizensWindow;
WINDOW* pProgressionWindow;

#define HOUSE_COLOR 4
#define HOSPITAL_COLOR 5
#define WASTELAND_COLOR 6
#define FIRE_STATION_COLOR 7

void initDataCollector()
{
    pFile = fopen("evolution.txt", "w");

    if (!pFile)
    {
        printf("Cannot open file\n");
        exit(EXIT_FAILURE);
    }

    saveMap(pFile);

    initscr();

    start_color();

    init_pair(1,COLOR_WHITE,COLOR_BLUE);

    init_pair(2,COLOR_BLUE,COLOR_WHITE);

    init_pair(3,COLOR_RED,COLOR_WHITE);

    init_pair(HOUSE_COLOR, COLOR_WHITE, COLOR_GREEN);
    init_pair(HOSPITAL_COLOR, COLOR_WHITE, COLOR_BLUE);
    init_pair(WASTELAND_COLOR, COLOR_WHITE, COLOR_YELLOW);
    init_pair(FIRE_STATION_COLOR, COLOR_WHITE, COLOR_RED);

    curs_set(0);

    noecho();

    pLegendWindow = subwin(stdscr, 9, 22, 2, 2);
    pTilesWindow = subwin(stdscr, 9, 15, 2, 27);
    pCitizensWindow = subwin(stdscr, 9, 18, 2, 45);
    pProgressionWindow = subwin(stdscr, 15, 30, 35, 2);
}

int getColor(char type)
{
    switch (type)
    {
    case HOSPITAL:
        return HOSPITAL_COLOR;
    
    case FIRE_STATION:
        return FIRE_STATION_COLOR;

    case HOUSE:
        return HOUSE_COLOR;

    case WASTELAND:
        return WASTELAND_COLOR;
    
    default:
        return 0;
    }
}

void updateDataCollector()
{
    clear();

    box(pLegendWindow, ACS_VLINE, ACS_HLINE);
    box(pTilesWindow, ACS_VLINE, ACS_HLINE);
    box(pCitizensWindow, ACS_VLINE, ACS_HLINE);

    wbkgd(pLegendWindow, COLOR_PAIR(2));

    wattron(pLegendWindow, A_UNDERLINE);
    mvwprintw(pLegendWindow, 0, 0, "Legende");
    wattroff(pLegendWindow, A_UNDERLINE);

    wattron(pLegendWindow, COLOR_PAIR(HOUSE_COLOR));
    mvwprintw(pLegendWindow, 2, 0, "  ");
    wattroff(pLegendWindow, COLOR_PAIR(HOUSE_COLOR));

    wattron(pLegendWindow, COLOR_PAIR(HOSPITAL_COLOR));
    mvwprintw(pLegendWindow, 4, 0, "  ");
    wattroff(pLegendWindow, COLOR_PAIR(HOSPITAL_COLOR));

    wattron(pLegendWindow, COLOR_PAIR(WASTELAND_COLOR));
    mvwprintw(pLegendWindow, 6, 0, "  ");
    wattroff(pLegendWindow, COLOR_PAIR(WASTELAND_COLOR));

    wattron(pLegendWindow, COLOR_PAIR(FIRE_STATION_COLOR));
    mvwprintw(pLegendWindow, 8, 0, "  ");
    wattroff(pLegendWindow, COLOR_PAIR(FIRE_STATION_COLOR));

    mvwprintw(pLegendWindow, 2, 3, "Maison");
    mvwprintw(pLegendWindow, 4, 3, "Hopital");
    mvwprintw(pLegendWindow, 6, 3, "Terrain vague");
    mvwprintw(pLegendWindow, 8, 3, "Caserne de pompiers");

    wbkgd(pTilesWindow, COLOR_PAIR(2));

    wattron(pTilesWindow, A_UNDERLINE);
    mvwprintw(pTilesWindow, 0, 0, "Carte des lieux");
    wattroff(pTilesWindow, A_UNDERLINE);

    for (int x = 0; x < MAP_WIDTH; x++)
    {
        for (int y = 0; y < MAP_HEIGHT; y++)
        {
            const Tile tile = getTile(x, y);

            wattron(pTilesWindow,COLOR_PAIR(getColor(tile.type)));

            mvwprintw(pTilesWindow, 2 + y, 2 * x, "  ");
            //mvwprintw(pTilesWindow, 2 + 2 * y + 1, 3 * x, "   ");

            wattroff(pTilesWindow,COLOR_PAIR(getColor(tile.type)));
        }
        
    }

    wbkgd(pCitizensWindow, COLOR_PAIR(2));

    wattron(pCitizensWindow, A_UNDERLINE);
    mvwprintw(pCitizensWindow, 0, 0, "Carte des citoyens");
    wattroff(pCitizensWindow, A_UNDERLINE);

    for (int x = 0; x < MAP_WIDTH; x++)
    {
        for (int y = 0; y < MAP_HEIGHT; y++)
        {
            int count = getCitizenCount(x, y);

            mvwprintw(pCitizensWindow, 2 + y, 2 * x, "%d ", count);
        }
    }

    wrefresh(pLegendWindow);
    wrefresh(pTilesWindow);
    wrefresh(pCitizensWindow);

    saveMap(pFile);
}

void destroyDataCollector()
{
    fclose(pFile);

    getch();
    endwin();

    free(pLegendWindow);
    free(pTilesWindow);
    free(pCitizensWindow);
    free(pProgressionWindow);
}