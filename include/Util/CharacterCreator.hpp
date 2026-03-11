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

        byte a[8] = {
            0b11111,
            0b11111,
            0b11111,
            0b00000,
            0b11111,
            0b11111,
            0b11111,
            0b00000};
        lcd->RegisterCustom("a", a);
    }
};