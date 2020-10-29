#!/usr/bin/env python3

"""
Sample Python/Pygame Programs
Simpson College Computer Science
http://programarcadegames.com/
http://simpson.edu/computer-science/
"""

import pygame
#import pygame.locals
import OpenGL.GL as GL
import OpenGL.GLU as GLU
import numpy as np

import bspline

# Define some colors
BLACK = (0, 0, 0)
WHITE = (255, 255, 255)
GREEN = (0, 255, 0)
RED = (255, 0, 0)
BLUE = (0, 0, 255)


#def draw_stick_figure(screen, x, y):
#  # Head
#  pygame.draw.ellipse(screen, BLACK, [1 + x, y, 10, 10], 0)
#
#  # Legs
#  pygame.draw.line(screen, BLACK, [5 + x, 17 + y], [10 + x, 27 + y], 2)
#  pygame.draw.line(screen, BLACK, [5 + x, 17 + y], [x, 27 + y], 2)
#
#  # Body
#  pygame.draw.line(screen, RED, [5 + x, 17 + y], [5 + x, 7 + y], 2)
#
#  # Arms
#  pygame.draw.line(screen, RED, [5 + x, 7 + y], [9 + x, 17 + y], 2)
#  pygame.draw.line(screen, RED, [5 + x, 7 + y], [1 + x, 17 + y], 2)

def draw_line(color, vertices):
  GL.glColor(color)
  GL.glBegin(GL.GL_LINES)
  for vertex in vertices:
    GL.glVertex3fv(vertex)
  GL.glEnd()


class Backboard():
  def __init__(self):
    self.nu = 10
    self.nv = 15
    self.us = np.linspace(0, 1, self.nu)
    self.vs = np.linspace(0, 1, self.nv)

    self.npx = 5
    self.npy = 4

    x0 = -1.
    xf =  1.
    z0 = -0.5
    zf =  0.5

    xs = np.linspace(x0, xf, self.npx)
    zs = np.linspace(z0, zf, self.npy)

    def y(x,z):
      return 0.4 * (x**2 + 2*z**2)

    self.ps = np.array([[(x, y(x, z), z) for x in xs] for z in zs])

    self.bspline = np.array(
      [[bspline.cubic_clamped_bspline2(u, v, self.ps)
        for v in self.vs]
       for u in self.us])

  def draw_quads(self):
    def draw_vertex(kx, ky):
      x, y, z = self.bspline[kx, ky, :]
      GL.glVertex3fv((x, y, z))

    #GL.glColor(WHITE)
    #GL.glBegin(GL.GL_QUADS)
    #
    #for kx in range(self.nu-1):
    #  for ky in range(self.nv-1):
    #    draw_vertex(kx, ky)
    #    draw_vertex(kx, ky+1)
    #    draw_vertex(kx+1, ky+1)
    #    draw_vertex(kx+1, ky)
    #GL.glEnd()

    # fill in grid
    GL.glColor(WHITE)
    GL.glBegin(GL.GL_LINES)

    for kx in range(self.nu-1):
      for ky in range(self.nv-1):
        draw_vertex(kx, ky)
        draw_vertex(kx+1, ky+1)

        draw_vertex(kx+1, ky+1)
        draw_vertex(kx+1, ky)

        draw_vertex(kx+1, ky)
        draw_vertex(kx, ky)

        # other
        draw_vertex(kx, ky)
        draw_vertex(kx+1, ky+1)

        draw_vertex(kx+1, ky+1)
        draw_vertex(kx, ky+1)

        draw_vertex(kx, ky+1)
        draw_vertex(kx, ky)

    GL.glEnd()


def draw_axes():
  draw_line(RED,   [(0,0,0), (1,0,0)])
  draw_line(GREEN, [(0,0,0), (0,1,0)])
  draw_line(BLUE,  [(0,0,0), (0,0,1)])

def main():
  backboard = Backboard()

  pygame.init()
  display = (800,600)
  screen = pygame.display.set_mode(display, pygame.DOUBLEBUF|pygame.OPENGL)
  pygame.display.set_caption("yolo")
  GLU.gluPerspective(45, display[0]/display[1], 1e-3, 1e4)

  # Loop until the user clicks the close button.
  done = False

  # Used to manage how fast the screen updates
  clock = pygame.time.Clock()

  # Hide the mouse cursor
  #pygame.mouse.set_visible(0)
  # -------- Main Program Loop -----------
  azimuth_deg = -50.
  elevation_deg = 20.
  dragging = False
  drag_start = None
  camera_distance = 5.0

  while not done:
    # ALL EVENT PROCESSING SHOULD GO BELOW THIS COMMENT
    for event in pygame.event.get():
      #print(event)
      if event.type == pygame.MOUSEBUTTONDOWN and event.button == 1:
        drag_start = event.pos
        dragging = True
      if event.type == pygame.MOUSEMOTION and dragging:
        pos = event.pos
        azimuth_deg += 0.5*(pos[0] - drag_start[0])
        elevation_deg += 0.5*(pos[1] - drag_start[1])
        if elevation_deg > 89:
          elevation_deg = 89
        if elevation_deg < -89:
          elevation_deg = -89
        if azimuth_deg > 180:
          azimuth_deg -= 360
        if azimuth_deg <= -180:
          azimuth_deg += 360
        drag_start = pos
        print('azimuth {:.3f} deg, elevation {:.3f} deg'.format(azimuth_deg, elevation_deg))
      if event.type == pygame.MOUSEBUTTONUP and event.button == 1:
        dragging = False
        drag_start = None
      if event.type == pygame.MOUSEBUTTONDOWN and event.button == 4:
        camera_distance /= 1.1
        if camera_distance < 1e-3:
          camera_distance = 1e-3
        print('camera_distance {:.3f}'.format(camera_distance))
      if event.type == pygame.MOUSEBUTTONDOWN and event.button == 5:
        camera_distance *= 1.1
        if camera_distance > 1000:
          camera_distance = 1000
        print('camera_distance {:.3f}'.format(camera_distance))
      if event.type == pygame.QUIT:
        done = True
    # ALL EVENT PROCESSING SHOULD GO ABOVE THIS COMMENT

    # ALL GAME LOGIC SHOULD GO BELOW THIS COMMENT

    # Call draw stick figure function
    pos = pygame.mouse.get_pos()
    x = pos[0]
    y = pos[1]

    # ALL GAME LOGIC SHOULD GO ABOVE THIS COMMENT

    # ALL CODE TO DRAW SHOULD GO BELOW THIS COMMENT
    GL.glLoadIdentity()
    GL.glClear(GL.GL_COLOR_BUFFER_BIT|GL.GL_DEPTH_BUFFER_BIT)
    GLU.gluPerspective(45, display[0]/display[1], 1e-3, 1e4)
    azimuth = azimuth_deg * np.pi/180.
    elevation = elevation_deg * np.pi/180.
    GLU.gluLookAt(-camera_distance*np.cos(azimuth)*np.cos(elevation),
                  -camera_distance*np.sin(azimuth)*np.cos(elevation),
                  -camera_distance*np.sin(elevation),
                  0, 0, 0,
                  0, 0, -1)
    draw_axes()

    backboard.draw_quads()
    # First, clear the screen to white. Don't put other drawing commands
    # above this, or they will be erased with this command.
    #screen.fill(WHITE)
    #draw_stick_figure(screen, x, y)

    # ALL CODE TO DRAW SHOULD GO ABOVE THIS COMMENT

    # Go ahead and update the screen with what we've drawn.
    pygame.display.flip()

    # Limit to 20 frames per second
    clock.tick(60)

  # Close the window and quit.
  # If you forget this line, the program will 'hang'
  # on exit if running from IDLE.
  pygame.quit()

if __name__=='__main__':
  main()
