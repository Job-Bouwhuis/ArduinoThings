#include "Components/lcd.hpp"

class CharacterCreator
{
public:
    void Create(Components::Lcd *lcd)
    {
        byte heart[8] = {
            0b00000,
            0b01010,
            0b10101,
            0b10001,
            0b01010,
            0b00100,
            0b00000,
            0b00000};
        lcd->RegisterCustom("heart", heart);

        byte heartFilled[8] = {
            0b00000,
            0b01010,
            0b11111,
            0b11111,
            0b01110,
            0b00100,
            0b00000,
            0b00000};
        lcd->RegisterCustom("heartfilled", heartFilled);

        byte ghost[8] = {
            0b00000,
            0b11111,
            0b10101,
            0b11011,
            0b11111,
            0b10101,
            0b00000,
            0b00000};
        lcd->RegisterCustom("ghost", ghost);

        byte tree[8] = {
            0b00000,
            0b00100,
            0b00100,
            0b01110,
            0b01110,
            0b11111,
            0b11111,
            0b00100};
        lcd->RegisterCustom("tree", tree);

        byte flower[8] = {
            0b00000,
            0b01010,
            0b00100,
            0b11111,
            0b00100,
            0b01010,
            0b00100,
            0b00000};
        lcd->RegisterCustom("flower", flower);

        byte shield[8] = {
            0b00100,
            0b01110,
            0b11111,
            0b11111,
            0b11111,
            0b01110,
            0b00100,
            0b00000};
        lcd->RegisterCustom("shield", shield);

        byte coin[8] = {
            0b00000,
            0b01110,
            0b10001,
            0b10101,
            0b10101,
            0b10001,
            0b01110,
            0b00000};
        lcd->RegisterCustom("coin", coin);

        byte bottle[8] = {
            0b00100,
            0b00100,
            0b01110,
            0b10001,
            0b10001,
            0b10001,
            0b01110,
            0b00000};
        lcd->RegisterCustom("bottle", bottle);

        byte sword[8] = {
            0b00100,
            0b00100,
            0b00100,
            0b00100,
            0b10101,
            0b01110,
            0b00100,
            0b00000};
        lcd->RegisterCustom("sword", sword);

        byte flame1[8] = {
            0b00100,
            0b00100,
            0b01100,
            0b01110,
            0b11110,
            0b01110,
            0b00100,
            0b00000};
        lcd->RegisterCustom("flame1", flame1);

        byte flame2[8] = {
            0b00100,
            0b01100,
            0b01110,
            0b11110,
            0b11110,
            0b01110,
            0b00100,
            0b00000};
        lcd->RegisterCustom("flame2", flame2);

        byte flame3[8] = {
            0b00100,
            0b00110,
            0b01110,
            0b11110,
            0b01110,
            0b01100,
            0b00100,
            0b00000};
        lcd->RegisterCustom("flame3", flame3);

        // Game3 custom characters for 4-line playfield
        // Empty + Empty
        byte empty_empty[8] = {
            0b00000,
            0b00000,
            0b00000,
            0b00000,
            0b00000,
            0b00000,
            0b00000,
            0b00000};
        lcd->RegisterCustom("ee", empty_empty);

        // Empty + Player (O)
        byte empty_player[8] = {
            0b00000,
            0b00000,
            0b00000,
            0b00000,
            0b01110,
            0b10001,
            0b10001,
            0b01110};
        lcd->RegisterCustom("ep", empty_player);

        // Empty + Obstacle (#)
        byte empty_obstacle[8] = {
            0b00000,
            0b00000,
            0b00000,
            0b00000,
            0b11111,
            0b11111,
            0b11111,
            0b11111};
        lcd->RegisterCustom("eo", empty_obstacle);

        // Player + Empty
        byte player_empty[8] = {
            0b01110,
            0b10001,
            0b10001,
            0b01110,
            0b00000,
            0b00000,
            0b00000,
            0b00000};
        lcd->RegisterCustom("pe", player_empty);

        // Player + Obstacle
        byte player_obstacle[8] = {
            0b01110,
            0b10001,
            0b10001,
            0b01110,
            0b11111,
            0b11111,
            0b11111,
            0b11111};
        lcd->RegisterCustom("po", player_obstacle);

        // Obstacle + Empty
        byte obstacle_empty[8] = {
            0b11111,
            0b11111,
            0b11111,
            0b11111,
            0b00000,
            0b00000,
            0b00000,
            0b00000};
        lcd->RegisterCustom("oe", obstacle_empty);

        // Obstacle + Player
        byte obstacle_player[8] = {
            0b11111,
            0b11111,
            0b11111,
            0b11111,
            0b01110,
            0b10001,
            0b10001,
            0b01110};
        lcd->RegisterCustom("op", obstacle_player);

        // Obstacle + Obstacle
        byte obstacle_obstacle[8] = {
            0b11111,
            0b11111,
            0b11111,
            0b11111,
            0b11111,
            0b11111,
            0b11111,
            0b11111};
        lcd->RegisterCustom("oo", obstacle_obstacle);
    }
};
