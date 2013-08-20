/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package calibrate;

import java.awt.Color;
import java.awt.Component;
import java.awt.Dimension;
import java.awt.Graphics;
import javax.swing.JPanel;

/**
 *
 * @author Caio
 */
class ImagePanel extends JPanel{
   ImagePanel() {
        // set a preferred size for the custom panel.
        setPreferredSize(new Dimension(420,420));
    }

   @Override
    public void paintComponent(Graphics g) {
        super.paintComponent(g);

        g.drawString("BLAH", 20, 20);
        g.drawRect(200, 200, 200, 200);
    }

}
