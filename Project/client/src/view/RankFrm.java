/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package view;
import controller.Client;
import java.io.IOException;
import java.util.ArrayList;
import java.util.List;
import java.util.Vector;
import java.util.logging.Level;
import java.util.logging.Logger;
import javax.swing.ImageIcon;
import javax.swing.JOptionPane;
import static javax.swing.WindowConstants.DISPOSE_ON_CLOSE;
import javax.swing.table.DefaultTableModel;
import model.User;

/**
 *
 * @author Admin
 */
public class RankFrm extends javax.swing.JFrame {
  private DefaultTableModel tableModel;
  private List<User> listPlayers;
  private List<String> rankSrc;
  /**
   * Creates new form RankFrm
   */
  public RankFrm() {
    initComponents();
    this.setTitle("Caro Master");
    tableModel = (DefaultTableModel) jTable1.getModel();
    this.setIconImage(new ImageIcon("assets/image/caroicon.png").getImage());
    this.setResizable(false);
    this.setDefaultCloseOperation(DISPOSE_ON_CLOSE);
    this.setLocationRelativeTo(null);
    rankSrc = new ArrayList<>();
    rankSrc.add("rank-gold");
    rankSrc.add("rank-sliver");
    rankSrc.add("bronze-rank");
    for(int i = 0; i < 8; i++){
      rankSrc.add("nomal-rank");
    }
    try {
      Client.socketHandle.write(Client.socketHandle.requestify("RANK", 0, "player_id=" + Client.user.getID(), ""));
    } catch (IOException ex) {
      JOptionPane.showMessageDialog(rootPane, ex.getMessage());
    }
  }

  public void setDataToTable(List<User> users){
    listPlayers = users;
    tableModel.setRowCount(0);
//    tableModel.addRow(new Object[]{"Thứ tự", "Username", "Avatar", "win", "loss", "points", "rank"});
    int i = 0;
    for(User user : listPlayers){
      tableModel.addRow(new Object[]{
        i + 1,
        user.getUsername(),
        new ImageIcon(user.getAvatar()),
        user.getnumberOfWin(),
        user.getNumberOfLoss(),
        user.getPoints(),
        new ImageIcon("assets/icon/" + rankSrc.get(i) + ".png")
      });
      i++;
    }
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
    jScrollPane1 = new javax.swing.JScrollPane();
    Object[][] rows = {
    };
    String[] columns = {"Thứ tự", "Username", "Avatar", "win", "loss", "points", "rank"};
    DefaultTableModel model = new DefaultTableModel(rows, columns){
      @Override
      public Class<?> getColumnClass(int column){
        switch(column){
          case 0: return String.class;
          case 1: return String.class;
          case 2: return ImageIcon.class;
          case 3: return String.class;
          case 4: return String.class;
          case 5: return String.class;
          case 6: return ImageIcon.class;
          default: return Object.class;
        }
      }
    };
    jTable1 = new javax.swing.JTable();

    setDefaultCloseOperation(javax.swing.WindowConstants.EXIT_ON_CLOSE);
    setBackground(new java.awt.Color(255, 255, 255));

    jPanel1.setBackground(new java.awt.Color(81, 81, 104));

    jLabel1.setFont(new java.awt.Font("Tahoma", 1, 18)); // NOI18N
    jLabel1.setForeground(new java.awt.Color(255, 255, 255));
    jLabel1.setHorizontalAlignment(javax.swing.SwingConstants.CENTER);
    jLabel1.setText("Bảng xếp hạng");

    jTable1.setFont(new java.awt.Font("Tahoma", 1, 14)); // NOI18N
    jTable1.setModel(model);
    jTable1.setFillsViewportHeight(true);
    jTable1.setRowHeight(62);
    jTable1.addMouseListener(new java.awt.event.MouseAdapter() {
      public void mouseClicked(java.awt.event.MouseEvent evt) {
        jTable1MouseClicked(evt);
      }
    });
    jScrollPane1.setViewportView(jTable1);

    javax.swing.GroupLayout jPanel1Layout = new javax.swing.GroupLayout(jPanel1);
    jPanel1.setLayout(jPanel1Layout);
    jPanel1Layout.setHorizontalGroup(
      jPanel1Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
      .addComponent(jLabel1, javax.swing.GroupLayout.DEFAULT_SIZE, 654, Short.MAX_VALUE)
      .addGroup(javax.swing.GroupLayout.Alignment.TRAILING, jPanel1Layout.createSequentialGroup()
        .addContainerGap()
        .addComponent(jScrollPane1)
        .addContainerGap())
    );
    jPanel1Layout.setVerticalGroup(
      jPanel1Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
      .addGroup(jPanel1Layout.createSequentialGroup()
        .addGap(14, 14, 14)
        .addComponent(jLabel1, javax.swing.GroupLayout.PREFERRED_SIZE, 30, javax.swing.GroupLayout.PREFERRED_SIZE)
        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.UNRELATED)
        .addComponent(jScrollPane1, javax.swing.GroupLayout.DEFAULT_SIZE, 531, Short.MAX_VALUE)
        .addContainerGap())
    );

    javax.swing.GroupLayout layout = new javax.swing.GroupLayout(getContentPane());
    getContentPane().setLayout(layout);
    layout.setHorizontalGroup(
      layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
      .addComponent(jPanel1, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
    );
    layout.setVerticalGroup(
      layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
      .addComponent(jPanel1, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
    );

    pack();
  }// </editor-fold>//GEN-END:initComponents

    private void jTable1MouseClicked(java.awt.event.MouseEvent evt) {//GEN-FIRST:event_jTable1MouseClicked
      if(jTable1.getSelectedRow() == -1)
        return;
      if(listPlayers.get(jTable1.getSelectedRow()).getID() == Client.user.getID()){
        JOptionPane.showMessageDialog(rootPane, "Thứ hạng của bạn là " + (jTable1.getSelectedRow() + 1));
        return;
      }
      Client.openView(Client.View.COMPETITORINFO, listPlayers.get(jTable1.getSelectedRow()));
    }//GEN-LAST:event_jTable1MouseClicked


  // Variables declaration - do not modify//GEN-BEGIN:variables
  private javax.swing.JLabel jLabel1;
  private javax.swing.JPanel jPanel1;
  private javax.swing.JScrollPane jScrollPane1;
  private javax.swing.JTable jTable1;
  // End of variables declaration//GEN-END:variables
}
