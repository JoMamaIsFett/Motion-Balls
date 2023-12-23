import math
import colorsys
import pygame
import sys
import random

pygame.init()
screen = pygame.display.set_mode((0, 0), pygame.FULLSCREEN)
clock = pygame.time.Clock()

width = screen.get_width()
height = screen.get_height()
running = True
fps = 120
time_multiplier = 60 / fps

friction = 0.99
pull_distance = 400
pull_strength = 1

ball_list = []

reset_trigger = True


class Ball:
    def __init__(self, x, y):
        self.x = x
        self.y = y
        self.speed_x = 0
        self.speed_y = 0

    def move(self):
        self.x += self.speed_x * time_multiplier
        self.y += self. speed_y * time_multiplier
        if not 10 < self.x < width - 10:
            self.speed_x = -self.speed_x
        if not 10 < self.y < height - 10:
            self.speed_y = -self.speed_y
        self.speed_x *= friction
        self.speed_y *= friction
        self.x = min(width, max(0, self.x))
        self.y = min(height, max(0, self.y))

    def move_to(self, pos_x, pos_y, direction):
        x_distance = pos_x - self.x
        y_distance = pos_y - self.y
        distance = math.sqrt(x_distance ** 2 + y_distance ** 2)
        if distance == 0:
            distance = 0.0001
        if distance <= pull_distance:
            ratio = pull_strength / distance
            x_pull = x_distance * ratio
            y_pull = y_distance * ratio
            if direction == "+":
                self.speed_x += x_pull
                self.speed_y += y_pull
            elif direction == "-":
                self.speed_x -= x_pull
                self.speed_y -= y_pull

    def render(self):
        speed = math.sqrt(self.speed_x ** 2 + self.speed_y ** 2)

        hue = min(0.95, max(0.0, speed / 60))
        saturation = 1.0
        value = min(0.95, max(0.0, speed / 5))

        rgb = colorsys.hsv_to_rgb(hue, saturation, value)

        scaled_rgb = [int(val * 255) for val in rgb]
        pygame.draw.circle(screen, scaled_rgb, (self.x, self.y), 5)


def render():
    screen.fill((10, 10, 10))
    for ball in ball_list:
        ball.render()
    pygame.display.update()


def key_handler(keys):
    global reset_trigger
    global running
    if keys[pygame.K_ESCAPE]:
        running = False
    if keys[pygame.K_SPACE] and reset_trigger:
        ball_list.clear()
        generate_balls()
        reset_trigger = False
    elif not keys[pygame.K_SPACE] and not reset_trigger:
        reset_trigger = True


def mouse_handler():
    mouse_buttons = pygame.mouse.get_pressed()
    if mouse_buttons[0] == 1:
        mouse_x, mouse_y = pygame.mouse.get_pos()
        for ball in ball_list:
            ball.move_to(mouse_x, mouse_y, "+")
    elif mouse_buttons[2] == 1:
        mouse_x, mouse_y = pygame.mouse.get_pos()
        for ball in ball_list:
            ball.move_to(mouse_x, mouse_y, "-")


def move_balls():
    for ball in ball_list:
        ball.move()


def generate_balls():
    for x in range(1000):
        ball_list.append(Ball(random.randint(10, width - 10), random.randint(10, height - 10)))


generate_balls()

while running:
    for event in pygame.event.get():
        if event.type == pygame.QUIT:
            running = False

    mouse_handler()
    move_balls()

    key_handler(pygame.key.get_pressed())
    render()
    clock.tick(fps)

pygame.quit()
sys.exit()
