// public domain
import java.sql.*;
import org.lwjgl.*;
import org.lwjgl.opengl.*;

/**
Quick and dirty display of 3D cube using OpenGL
*/
public class Cube {
	static final int WIDTH = 800;
	static final int HEIGHT = 600;

	public static void main(String[] argv) {
		try {
			Display.setDisplayMode(new DisplayMode(WIDTH,HEIGHT));
			Display.create();
		} catch (LWJGLException e) {
			e.printStackTrace();
			System.exit(1);
		}

		// init OpenGL
		GL11.glMatrixMode(GL11.GL_PROJECTION);
		GL11.glLoadIdentity();
		GL11.glOrtho(0, WIDTH, 0, HEIGHT, 1, -1);
		GL11.glMatrixMode(GL11.GL_MODELVIEW);

		while (!Display.isCloseRequested()) {
			// Clear the screen and depth buffer
			GL11.glClear(GL11.GL_COLOR_BUFFER_BIT | GL11.GL_DEPTH_BUFFER_BIT); 

			// set the color of the quad (R,G,B,A)
			GL11.glColor3f(0.5f,0.5f,1.0f);

			// draw quad
			GL11.glBegin(GL11.GL_QUADS);
			GL11.glVertex2f(100,100);
			GL11.glVertex2f(100+200,100);
			GL11.glVertex2f(100+200,100+200);
			GL11.glVertex2f(100,100+200);
			GL11.glEnd();

			Display.update();
		}

		Display.destroy();
	}
}
