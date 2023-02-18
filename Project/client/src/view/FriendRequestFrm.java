/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package view;

import controller.Client;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.io.IOException;
import java.util.logging.Level;
import java.util.logging.Logger;
import javax.swing.ImageIcon;
import javax.swing.JOptionPane;
import javax.swing.Timer;
import static javax.swing.WindowConstants.DISPOSE_ON_CLOSE;

/**
 *
 * @author Admin
 */
public class FriendRequestFrm extends javax.swing.JFrame {
  private int ID;
  private Timer timer;
  private String type;
  /**
   * Creates new form FriendRequestFrm
   */
  public FriendRequestFrm(String type, int ID, String username) {
    this.ID = ID;
    this.type = type;
        
    
    initComponents();
    this.setTitle("Caro Master");
    if(type.equals("friend")) {
      jLabel1.setText("Yêu cầu kết bạn");
      jLabel2.setText("Bạn nhận được một lời mời kết bạn");
    }
    else if(type.equals("duel")) {
      jLabel1.setText("Yêu cầu thách đấu");
      jLabel2.setText("Bạn nhận được một lời mời thách đấu");
    }
    this.setIconImage(new ImageIcon("assets/image/caroicon.png").getImage());
    this.setResizable(false);
    this.setDefaultCloseOperation(DISPOSE_ON_CLOSE);
    this.setLocationRelativeTo(null);
    jLabel7.setText("Từ " + username + "(ID=" + ID + ")");
    timer = new Timer(1000, new ActionListener() {
      int count = 10;

      @Override
      public void actionPerformed(ActionEvent e) {
        count--;
        if (count >= 0) {
          jLabel3.setText("Tự động đóng trong " + count);
        } else {
          ((Timer) (e.getSource())).stop();
          if(type.equals("duel")) {
            try {
              Client.socketHandle.write(
                Client.socketHandle.requestify(
                  "DUEL", 0, "player_id=" + Client.user.getID() + "&friend_id=" + ID + "&agree=0", ""
                )
              );
            } catch (IOException ex) {
              Logger.getLogger(FriendRequestFrm.class.getName()).log(Level.SEVERE, null, ex);
            }
        }
          disposeFrame();
        }
      }
    });
    timer.setInitialDelay(0);
    timer.start();
  }
  public void disposeFrame(){
    this.dispose();
  }
  /**
   * This method is called from within the constructor to initialize the form.
   * WARNING: Do NOT modify this code. The content of this method is always
   * regenerated by the Form Editor.
   */
  @SuppressWarnings("unchecked")
  // <editor-fold defaultstate="collapsed" desc="Generated Code">//GEN-BEGIN:initComponents
  private void initComponents() {

    jPanel1 = new javax.swing.JPanel();
    jLabel1 = new javax.swing.JLabel();
    jLabel2 = new javax.swing.JLabel();
    jLabel7 = new javax.swing.JLabel();
    jButton1 = new javax.swing.JButton();
    jButton2 = new javax.swing.JButton();
    jLabel3 = new javax.swing.JLabel();

    setDefaultCloseOperation(javax.swing.WindowConstants.EXIT_ON_CLOSE);

    jPanel1.setBackground(new java.awt.Color(81, 81, 104));

    jLabel1.setFont(new java.awt.Font("Tahoma", 1, 18)); // NOI18N
    jLabel1.setForeground(new java.awt.Color(255, 255, 255));
    jLabel1.setHorizontalAlignment(javax.swing.SwingConstants.CENTER);
    jLabel1.setText("{Title}");

    javax.swing.GroupLayout jPanel1Layout = new javax.swing.GroupLayout(jPanel1);
    jPanel1.setLayout(jPanel1Layout);
    jPanel1Layout.setHorizontalGroup(
      jPanel1Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
      .addComponent(jLabel1, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
    );
    jPanel1Layout.setVerticalGroup(
      jPanel1Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
      .addGroup(jPanel1Layout.createSequentialGroup()
        .addGap(28, 28, 28)
        .addComponent(jLabel1)
        .addContainerGap(29, Short.MAX_VALUE))
    );

    jLabel2.setFont(new java.awt.Font("Tahoma", 1, 14)); // NOI18N
    jLabel2.setForeground(new java.awt.Color(0, 102, 204));
    jLabel2.setHorizontalAlignment(javax.swing.SwingConstants.CENTER);
    jLabel2.setText("{Heading}");

    jLabel7.setFont(new java.awt.Font("Tahoma", 1, 14)); // NOI18N
    jLabel7.setForeground(new java.awt.Color(0, 102, 204));
    jLabel7.setHorizontalAlignment(javax.swing.SwingConstants.CENTER);
    jLabel7.setText("Từ");

    jButton1.setBackground(new java.awt.Color(230, 246, 236));
    jButton1.setFont(new java.awt.Font("Liberation Sans", 1, 15)); // NOI18N
    jButton1.setForeground(new java.awt.Color(62, 179, 97));
    jButton1.setIcon(new javax.swing.ImageIcon("/home/fuurinkazan/Documents/C/Network Programming/Project/client/assets/icon/ok.png")); // NOI18N
    jButton1.setText("Đồng ý");
    jButton1.setBorder(javax.swing.BorderFactory.createLineBorder(new java.awt.Color(62, 179, 97)));
    jButton1.addActionListener(new java.awt.event.ActionListener() {
      public void actionPerformed(java.awt.event.ActionEvent evt) {
        jButton1ActionPerformed(evt);
      }
    });

    jButton2.setBackground(new java.awt.Color(251, 230, 230));
    jButton2.setFont(new java.awt.Font("Liberation Sans", 1, 15)); // NOI18N
    jButton2.setForeground(new java.awt.Color(215, 37, 3));
    jButton2.setIcon(new javax.swing.ImageIcon("/home/fuurinkazan/Documents/C/Network Programming/Project/client/assets/icon/close.png")); // NOI18N
    jButton2.setText("Từ chối");
    jButton2.setBorder(javax.swing.BorderFactory.createLineBorder(new java.awt.Color(215, 37, 3)));
    jButton2.addActionListener(new java.awt.event.ActionListener() {
      public void actionPerformed(java.awt.event.ActionEvent evt) {
        jButton2ActionPerformed(evt);
      }
    });

    jLabel3.setFont(new java.awt.Font("Liberation Sans", 1, 15)); // NOI18N
    jLabel3.setForeground(new java.awt.Color(81, 81, 104));
    jLabel3.setText("Tự động đóng sau ");

    javax.swing.GroupLayout layout = new javax.swing.GroupLayout(getContentPane());
    getContentPane().setLayout(layout);
    layout.setHorizontalGroup(
      layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
      .addComponent(jPanel1, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
      .addComponent(jLabel2, javax.swing.GroupLayout.Alignment.TRAILING, javax.swing.GroupLayout.DEFAULT_SIZE, 360, Short.MAX_VALUE)
      .addComponent(jLabel7, javax.swing.GroupLayout.Alignment.TRAILING, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
      .addGroup(layout.createSequentialGroup()
        .addGap(38, 38, 38)
        .addComponent(jButton1, javax.swing.GroupLayout.PREFERRED_SIZE, 126, javax.swing.GroupLayout.PREFERRED_SIZE)
        .addGap(32, 32, 32)
        .addComponent(jButton2, javax.swing.GroupLayout.PREFERRED_SIZE, 127, javax.swing.GroupLayout.PREFERRED_SIZE)
        .addGap(0, 0, Short.MAX_VALUE))
      .addGroup(layout.createSequentialGroup()
        .addGap(169, 169, 169)
        .addComponent(jLabel3, javax.swing.GroupLayout.DEFAULT_SIZE, 185, Short.MAX_VALUE)
        .addContainerGap())
    );
    layout.setVerticalGroup(
      layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
      .addGroup(layout.createSequentialGroup()
        .addComponent(jPanel1, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
        .addGap(18, 18, 18)
        .addComponent(jLabel2)
        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
        .addComponent(jLabel7)
        .addGap(18, 18, 18)
        .addGroup(layout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
          .addComponent(jButton1, javax.swing.GroupLayout.PREFERRED_SIZE, 40, javax.swing.GroupLayout.PREFERRED_SIZE)
          .addComponent(jButton2, javax.swing.GroupLayout.PREFERRED_SIZE, 40, javax.swing.GroupLayout.PREFERRED_SIZE))
        .addGap(18, 18, 18)
        .addComponent(jLabel3)
        .addContainerGap(20, Short.MAX_VALUE))
    );

    pack();
  }// </editor-fold>//GEN-END:initComponents

    private void jButton1ActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_jButton1ActionPerformed
      timer.stop();
      if(this.type.equals("friend")) {
        try {
          Client.socketHandle.write(
            Client.socketHandle.requestify(
              "FRIEND_ACCEPT", 0, "player_id=" + Client.user.getID() + "&friend_id=" + ID, ""
            )
          );
        } catch (IOException ex) {
          JOptionPane.showMessageDialog(rootPane, "Có lỗi xảy ra");
        }
      }
      else if(this.type.equals("duel")) {
        try {
          Client.socketHandle.write(
            Client.socketHandle.requestify("DUEL", 0, "player_id=" + Client.user.getID() + "&friend_id=" + this.ID + "&agree=1", "")
          );
          
        } catch (IOException ex) {
          JOptionPane.showMessageDialog(rootPane, "Có lỗi xảy ra");
        }
      }
      this.dispose();
    }//GEN-LAST:event_jButton1ActionPerformed

    private void jButton2ActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_jButton2ActionPerformed
      timer.stop();
      if(this.type.equals("duel")) {
        try {
          Client.socketHandle.write(
            Client.socketHandle.requestify(
              "DUEL", 0, "player_id=" + Client.user.getID() + "&friend_id=" + this.ID + "&agree=0", ""
            )
          );
        } catch (IOException ex) {
          JOptionPane.showMessageDialog(rootPane, "Có lỗi xảy ra");
        }
      }
      this.dispose();
    }//GEN-LAST:event_jButton2ActionPerformed

    /**
     * @param args the command line arguments
     */

  // Variables declaration - do not modify//GEN-BEGIN:variables
  private javax.swing.JButton jButton1;
  private javax.swing.JButton jButton2;
  private javax.swing.JLabel jLabel1;
  private javax.swing.JLabel jLabel2;
  private javax.swing.JLabel jLabel3;
  private javax.swing.JLabel jLabel7;
  private javax.swing.JPanel jPanel1;
  // End of variables declaration//GEN-END:variables
}
