import math
import colorsys
import pygame
import sys
import random

# you can change these values

BACKGROUND_COLOR = (10, 10, 10)
MENU_COLOR_MAIN = (210, 210, 210)
MENU_COLOR_SECONDARY = (100, 100, 100)

friction = 0.99
pull_distance = 400
pull_strength = 1
ball_number = 1000

min_max = [(0.9, 1.0), (200, 800), (0.5, 5), (100, 10000)]

slider_click_range = 10


# don't change these values

pygame.init()
screen = pygame.display.set_mode((0, 0), pygame.FULLSCREEN)
clock = pygame.time.Clock()
font_32 = pygame.font.Font(None, 32)
font_64 = pygame.font.Font(None, 64)

width = screen.get_width()
height = screen.get_height()
running = True
in_menu = False
fps = 120

ball_list = []

reset_trigger = True
menu_trigger = True

menu_slider = [(width / 2, height / 2 - 300), (width / 2, height / 2 - 100), (width / 2, height / 2 + 100),
               (width / 2, height / 2 + 300)]
menu_options = ["Friction", "Pull distance", "Pull strength", "Ball number"]
slider_width = width / 3
menu_rects = []
for sl in menu_slider:
    x_s, y_s = sl
    x_left = x_s - slider_width / 2
    y_left = y_s - 4 - slider_click_range
    h_r = 8 + 2 * slider_click_range
    menu_rects.append(pygame.Rect(x_left, y_left, slider_width, h_r))

quit_rect = pygame.Rect(width * (9/10), 20, width * (1/10) - 20, height * (1/10) - 20)


class Ball:
    def __init__(self, x, y):
        self.x = x
        self.y = y
        self.speed_x = 0
        self.speed_y = 0

    def move(self):
        self.x += self.speed_x * 0.5
        self.y += self.speed_y * 0.5
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


def value_to_pos(value, width_bar, min_value, max_value):
    area = max_value - min_value
    step = width_bar / area
    value -= min_value
    pos = value * step
    return pos


def pos_to_value(x_value, width_bar, min_value, max_value):
    area = max_value - min_value
    step = area / width_bar
    value = x_value * step + min_value
    return value


def render():
    screen.fill(BACKGROUND_COLOR)
    if in_menu:
        render_menu()
    else:
        for ball in ball_list:
            ball.render()
    pygame.display.flip()


def render_menu():
    item_list = [friction, pull_distance, pull_strength, ball_number]
    for slider in range(len(menu_slider)):
        # pygame.draw.rect(screen, (50, 50, 50), menu_rects[slider])
        mini, maxi = min_max[slider]
        render_bar(menu_slider[slider], item_list[slider], mini, maxi)
    render_text(item_list)

    pygame.draw.rect(screen, MENU_COLOR_SECONDARY, quit_rect)
    quit_text = font_64.render("Quit", True, MENU_COLOR_MAIN)
    quit_text_rect = quit_text.get_rect()
    quit_text_rect.center = quit_rect.center
    screen.blit(quit_text, quit_text_rect)


def render_bar(pos, item, mini, maxi, w=slider_width, h=8):
    x, y = pos

    x_l = x - (w / 2)
    y_l = y - (h / 2)

    r = h / 2
    m_l = x_l
    m_r = x_l + w

    pygame.draw.rect(screen, MENU_COLOR_SECONDARY, pygame.Rect(x_l, y_l, w, h))
    pygame.draw.circle(screen, MENU_COLOR_SECONDARY, (m_l, y), r)
    pygame.draw.circle(screen, MENU_COLOR_SECONDARY, (m_r, y), r)

    x_value = value_to_pos(item, w, mini, maxi)
    x_pos = x_l + x_value
    pygame.draw.rect(screen, MENU_COLOR_MAIN, pygame.Rect(x_pos - r, y_l, h, h))
    pygame.draw.circle(screen, MENU_COLOR_MAIN, (x_pos, y_l), r)
    pygame.draw.circle(screen, MENU_COLOR_MAIN, (x_pos, y_l + h), r)


def render_text(item_list):
    for item in range(len(item_list)):
        x, y = menu_slider[item]

        item_number_text = font_32.render(str(round(item_list[item], 4)), True, MENU_COLOR_MAIN)
        item_number_rect = item_number_text.get_rect()
        item_number_rect.midleft = (x + slider_width / 2 + 10, y)
        screen.blit(item_number_text, item_number_rect)

        item_name_text = font_32.render(menu_options[item], True, MENU_COLOR_MAIN)
        item_name_rect = item_name_text.get_rect()
        item_name_rect.center = (x, y - 30)
        screen.blit(item_name_text, item_name_rect)


def key_handler(keys):
    global reset_trigger, in_menu, menu_trigger
    if keys[pygame.K_ESCAPE] and menu_trigger:
        in_menu = not in_menu
        menu_trigger = False
    elif not keys[pygame.K_ESCAPE] and not menu_trigger:
        menu_trigger = True
    if keys[pygame.K_SPACE] and reset_trigger:
        generate_balls()
        reset_trigger = False
    elif not keys[pygame.K_SPACE] and not reset_trigger:
        reset_trigger = True


def menu_click():
    global friction, pull_distance, pull_strength, ball_number, running
    mouse_pos = pygame.mouse.get_pos()
    x_mouse, y_mouse = mouse_pos
    for rect in menu_rects:
        if rect.collidepoint(mouse_pos):
            index = menu_rects.index(rect)
            x_pos = x_mouse - menu_rects[index].left
            mini, maxi = min_max[index]
            value = pos_to_value(x_pos, slider_width, mini, maxi)
            if index == 0:
                friction = value
            elif index == 1:
                pull_distance = value
            elif index == 2:
                pull_strength = value
            elif index == 3:
                ball_number = int(value)
                generate_balls()
    if quit_rect.collidepoint(mouse_pos):
        running = False


def mouse_handler():
    mouse_buttons = pygame.mouse.get_pressed()
    if not in_menu:
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
    ball_list.clear()
    for x in range(ball_number):
        ball_list.append(Ball(random.randint(10, width - 10), random.randint(10, height - 10)))


generate_balls()

while running:
    for event in pygame.event.get():
        if event.type == pygame.QUIT:
            running = False
        if event.type == pygame.MOUSEBUTTONDOWN and in_menu:
            menu_click()

    mouse_handler()
    if not in_menu:
        move_balls()

    key_handler(pygame.key.get_pressed())
    render()
    clock.tick(fps)

pygame.quit()
sys.exit()
