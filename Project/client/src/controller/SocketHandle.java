/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package controller;
import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.OutputStreamWriter;
import java.net.Socket;
import java.net.UnknownHostException;
import java.util.ArrayList;
import java.util.List;
import java.util.Vector;
import java.util.regex.Matcher;
import java.util.regex.Pattern;
import javax.swing.JOptionPane;
import model.User;
import model.Response;

/**
 *
 * @author Admin
 */
public class SocketHandle implements Runnable {
  private BufferedWriter os;
  private BufferedReader is;
  private Socket socketOfClient;
  private int ID_Server;
  
  public List<User> getListUser(String[] message){
    Pattern p = Pattern.compile("id=(\\d+),username=([a-zA-Z0-9]+),avatar=([a-zA-Z0-9/\\.]+),is_online=([a-z]+),is_playing=([a-z]+)");
    Matcher m;
    List<User> friend = new ArrayList<>();
    for(int i = 0; i < message.length; i++){
      m = p.matcher(message[i]);
      if(m.find())
        friend.add(new User(
          Integer.parseInt(m.group(1)), 
          m.group(2), 
          m.group(3), 
          Boolean.parseBoolean(m.group(4)), 
          Boolean.parseBoolean(m.group(5))
        ));
    }
    return friend;
  }
  
  public List<User> getListRank(String data){
    String[] splitter = data.split(";");
    List<User> player = new ArrayList<>();
    Pattern pattern = Pattern.compile("id=(\\d+),username=([a-zA-Z0-9]+),avatar=([a-zA-Z0-9/\\.]+),win=(\\d+),loss=(\\d+),points=(-?\\d+)");
    Matcher m;
    for(int i = 0; i < splitter.length; i++){
      m = pattern.matcher(splitter[i]);
      if(m.find()) {
        player.add(new User(
            Integer.parseInt(m.group(1)), 
            m.group(2), 
            m.group(3), 
            Integer.parseInt(m.group(4)), 
            Integer.parseInt(m.group(5)), 
            Integer.parseInt(m.group(6))
          )
        );
      }
    }
    return player;
  }
  
  // data: id=...,username=...,password=...,avatar=...,game=...,win=...,draw=...,loss=...,points=...,rank=...
  public User getUserFromString(String data){
    Pattern pattern = Pattern.compile(
      "id=(\\d+),username=([a-zA-Z0-9]+),password=([a-zA-Z0-9]+),avatar=([a-zA-Z0-9/\\.]+),"
          + "game=(\\d+),win=(\\d+),draw=(\\d+),loss=(\\d+),points=(\\d+),rank=(\\d+),is_online=([a-z]+),is_playing=([a-z]+)"
    );
    Matcher m = pattern.matcher(data);
    m.find();
    return new User(
      Integer.parseInt(m.group(1)), 
      m.group(2), 
      m.group(3), 
      m.group(4), 
      Integer.parseInt(m.group(5)), 
      Integer.parseInt(m.group(6)), 
      Integer.parseInt(m.group(7)), 
      Integer.parseInt(m.group(8)), 
      Integer.parseInt(m.group(9)), 
      Integer.parseInt(m.group(10)),
      Boolean.parseBoolean(m.group(11)),
      Boolean.parseBoolean(m.group(12))
    );
  }
  
  @Override
  public void run() {
    try {
      // Gửi yêu cầu kết nối tới Server đang lắng nghe
      socketOfClient = new Socket("0.tcp.ap.ngrok.io", 10874);
      System.out.println("Kết nối thành công!");
      
      // Tạo luồng đầu ra tại client (Gửi dữ liệu tới server)
      os = new BufferedWriter(new OutputStreamWriter(socketOfClient.getOutputStream()));
      // Luồng đầu vào tại Client (Nhận dữ liệu từ server).
      is = new BufferedReader(new InputStreamReader(socketOfClient.getInputStream()));
      String message;
      Response res;
      
      while (true) {
        // Nhận response từ server
        message = is.readLine();
        if (message == null) {
          break;
        }
        System.out.println("Server response: " + message);
        res = new Response(message);
        
//        String[] messageSplit = message.split(",");
//        if(messageSplit[0].equals("server-send-id")){
//          ID_Server = Integer.parseInt(messageSplit[1]);
//        }

        
        /* ---------------------------------------------------------------------------------- */
        /*                                AUTHENTICATION                                      */
        /* ---------------------------------------------------------------------------------- */
        
        // Đăng nhập thành công
        if(res.getState().equals("login_success") || res.getState().equals("register_success")){
          System.out.println("Đăng nhập thành công");
          Client.closeAllViews();
          User user = getUserFromString(res.getData());
          Client.user = user;
          Client.openView(Client.View.HOMEPAGE);
        }
        
        // Thông tin tài khoản sai
        if(res.getState().equals("account_incorrect")){
          Client.closeView(Client.View.GAMENOTICE);
          Pattern pattern = Pattern.compile("username=([a-zA-Z0-9]+),password=([a-zA-Z0-9]+)");
          Matcher m = pattern.matcher(res.getData());
          m.find();
          Client.openView(Client.View.LOGIN, m.group(1), m.group(2));
          Client.loginFrm.showError("Username hoặc mật khẩu không chính xác");
        }
        
        // Tài khoản đã đăng nhập ở nơi khác
        if(res.getState().equals("login_duplicate")){
          System.out.println("Đã đăng nhập");
          Client.closeView(Client.View.GAMENOTICE);          
          Pattern pattern = Pattern.compile("username=([a-zA-Z0-9]+),password=([a-zA-Z0-9]+)");
          Matcher m = pattern.matcher(res.getData());
          m.find();
          Client.openView(Client.View.LOGIN, m.group(1), m.group(2));
          Client.loginFrm.showError("Tài khoản đã đăng nhập ở nơi khác");
        }       
       
        // Xử lý register trùng tên
        if(res.getState().equals("username_duplicate")){
          Client.closeAllViews();
          Client.openView(Client.View.REGISTER);
          JOptionPane.showMessageDialog(Client.registerFrm, "Username đã được sử dụng");
        }
        
        // Xử lý tài khoản hoặc mật khẩu không hợp lệ 
        if(res.getState().equals("account_invalid")){
          Client.closeAllViews();
          Client.openView(Client.View.REGISTER);
          JOptionPane.showMessageDialog(Client.registerFrm, "Tài khoản và mật khẩu không hợp lệ");
        }
        
        
        /* ---------------------------------------------------------------------------------- */
        /*                                      CHAT                                          */
        /* ---------------------------------------------------------------------------------- */
 
        // Xử lý chat global
        if(res.getState().equals("chat_global")){
          if(Client.homePageFrm != null){
            Client.homePageFrm.addMessage(res.getData());
          }
        }
        
        // Xử lý chat trong game
        if(res.getState().equals("chat_local")){
          Client.gameClientFrm.addMessage(res.getData());
        }
        
        /* ---------------------------------------------------------------------------------- */
        /*                                      ROOM                                          */
        /* ---------------------------------------------------------------------------------- */
        
                
        // Tạo phòng và server trả về tên phòng
        if(res.getState().equals("game_created")){
          Client.closeAllViews();
          Client.openView(Client.View.WAITINGROOM);
          Pattern pattern = Pattern.compile("game_id=(\\d+)(,password=([a-zA-Z0-9]+))?");
          Matcher m = pattern.matcher(res.getData());
          m.find();
          Client.waitingRoomFrm.game_id = Integer.parseInt(m.group(1));
          Client.waitingRoomFrm.setRoomName(m.group(1));
          Client.waitingRoomFrm.setRoomPassword("Mật khẩu phòng: " + (m.group(2) == null ? "không có" : m.group(3)));
        }
        
        // Xử lý lấy danh sách phòng
        if(res.getState().equals("game_list")){
          Vector<String> rooms = new Vector<>();
          Vector<String> passwords = new Vector<>();
          String[] splitter = res.getData().split(";");
          Pattern p = Pattern.compile("game_id=(\\d+),password=([a-zA-Z0-9]+)?,num_move=(\\d+),player1_id=(\\d+),player2_id=(\\d+)");
          Matcher m;
          for (String splitter1 : splitter) {
            m = p.matcher(splitter1);
            if(m.find()) {
              rooms.add("Phòng " + m.group(1));
              passwords.add(m.group(2));
              System.out.println("Room: " + m.group(1) + " - Password: " + m.group(2));
            }
          }
          Client.roomListFrm.updateRoomList(rooms, passwords);
        }
        
        // Xử lý vào phòng đã đủ người chơi 
        if(res.getState().equals("game_full")){
          Client.closeAllViews();
          Client.openView(Client.View.HOMEPAGE);
          JOptionPane.showMessageDialog(Client.homePageFrm, "Phòng chơi đã đủ 2 người chơi");
        }
        
        // Xử lý không tìm thấy phòng trong chức năng vào phòng
        if(res.getState().equals("game_null")){
          Client.closeAllViews();
          Client.openView(Client.View.HOMEPAGE);
          JOptionPane.showMessageDialog(Client.homePageFrm, "Không tìm thấy phòng");
        }
    
        // Xử lý phòng có mật khẩu sai
        if(res.getState().equals("game_password_incorrect")){
          Client.closeAllViews();
          Client.openView(Client.View.HOMEPAGE);
          JOptionPane.showMessageDialog(Client.homePageFrm, "Mật khẩu phòng sai");
        }
     
        // Xử lý vào phòng. data: game_id=...,is_start=...,ip=...,id=...,username=...,password=...,avatar=...,game=...,win=...,draw=...,loss=...,points=...,rank=...
        if(res.getState().equals("game_joined")){
          String[] splitter = res.getData().split(";");
          Pattern p = Pattern.compile(
            "game_id=(\\d+),is_start=(\\d+),ip=([\\.\\d]+),"
            + "id=(\\d+),username=([a-zA-Z0-9]+),avatar=([a-zA-Z0-9/\\.]+),game=(\\d+),"
            + "win=(\\d+),draw=(\\d+),loss=(\\d+),points=(\\d+),rank=(\\d+)"
          );
          Matcher m;
          int opponentID, roomID = 0, isStart = 0;
          String competitorIP = "0.0.0.0";
          User competitor = new User(0, "");
          for (String splitter1 : splitter) {
            m = p.matcher(splitter1);
            if(m.find()) {
              opponentID = Integer.parseInt(m.group(4));
              
              if(opponentID != Client.user.getID()) {
                roomID = Integer.parseInt(m.group(1));
                isStart = Integer.parseInt(m.group(2));
                competitorIP = m.group(3);
                competitor = new User(
                  opponentID, m.group(5), "", m.group(6),
                  Integer.parseInt(m.group(7)), Integer.parseInt(m.group(8)),
                  Integer.parseInt(m.group(9)), Integer.parseInt(m.group(10)),
                  Integer.parseInt(m.group(11)), Integer.parseInt(m.group(12))
                );
              }
            }
          }
          System.out.println("Vào phòng");
          
          if(Client.findRoomFrm != null){
            Client.findRoomFrm.showFindedRoom();
            try {
              Thread.sleep(3000);
            } catch (InterruptedException ex) {
              JOptionPane.showMessageDialog(Client.findRoomFrm, "Lỗi khi sleep thread");
            }
          } 
          else if(Client.waitingRoomFrm != null){
            Client.waitingRoomFrm.showFindedCompetitor();
            try {
              Thread.sleep(3000);
            } catch (InterruptedException ex) {
              JOptionPane.showMessageDialog(Client.waitingRoomFrm, "Lỗi khi sleep thread");
            }
          }
          Client.closeAllViews();
          System.out.println("Đã vào phòng: " + roomID);
          
          Client.openView(Client.View.GAMECLIENT, competitor, roomID, isStart, competitorIP);
          Client.gameClientFrm.newgame();
        }
        
        // Xử lý rời phòng
        if(res.getState().equals("game_quit")){
          Client.gameClientFrm.stopTimer();
          Client.closeAllViews();
          Client.openView(Client.View.GAMENOTICE, "Đối thủ đã thoát khỏi phòng", "Đang trở về trang chủ");
          Thread.sleep(3000);       
          Client.closeAllViews();
          Client.openView(Client.View.HOMEPAGE);
        }

        
        /* ---------------------------------------------------------------------------------- */
        /*                                      FRIEND                                        */
        /* ---------------------------------------------------------------------------------- */
        
                        
        // Xử lý hiển thị thông tin đối thủ là bạn bè/không
        if(res.getState().equals("friend_check")){
          if(Client.competitorInfoFrm != null){
            Pattern p = Pattern.compile("is_friend=(\\d+)");
            Matcher m = p.matcher(res.getData());
            m.find();
            String isFriend = m.group(1);
            Client.competitorInfoFrm.checkFriend((isFriend.equals("1")));
          }
        }
        
        // Xử lý danh sách bạn bè 
        if(res.getState().equals("friend_list")){
          if(Client.friendListFrm != null){
            String[] splitter = res.getData().split(";");
            Client.friendListFrm.updateFriendList(getListUser(splitter));
          }
        }
        
        // Xử lý yêu cầu kết bạn tới
        if(res.getState().equals("friend_request")){
          Pattern p = Pattern.compile("player_id=(\\d+),username=([a-zA-Z0-9]+)");
          Matcher m = p.matcher(res.getData());
          m.find();
          int ID = Integer.parseInt(m.group(1));
          String username = m.group(2);
          Client.openView(Client.View.FRIENDREQUEST, ID, username);
        }
        
        // Xử lý xem rank
        if(res.getState().equals("rank")){
          if(Client.rankFrm != null){
            Client.rankFrm.setDataToTable(getListRank(res.getData()));
          }
        }
        
        /* ---------------------------------------------------------------------------------- */
        /*                                CHALLENGE REQUEST                                   */
        /* ---------------------------------------------------------------------------------- */

        // Xử lý khi nhận được yêu cầu thách đấu
        if(res.getState().equals("duel_request")){
          Pattern p = Pattern.compile("player_id=(\\d+),username=([a-zA-Z0-9]+)");
          Matcher m = p.matcher(res.getData());
          m.find();
          int ret = JOptionPane.showConfirmDialog(
            Client.getVisibleJFrame(), 
            "Bạn nhận được lời thách đấu của " + m.group(2) + " (ID=" + m.group(1) + ")", 
            "Xác nhận thách đấu", 
            JOptionPane.YES_NO_OPTION
          );
          if(ret == JOptionPane.YES_OPTION){
            Client.socketHandle.write(
              Client.socketHandle.requestify("DUEL", 0, "player_id=" + Client.user.getID() + "&friend_id=" + m.group(1) + "&agree=1", "")
            );
          }
          else{
            Client.socketHandle.write(
              Client.socketHandle.requestify("DUEL", 0, "player_id=" + Client.user.getID() + "&friend_id=" + m.group(1) + "&agree=0", "")
            );
          }
        }
        
        // Xử lý không đồng ý thách đấu
        if(res.getState().equals("duel_rejected")){
          Client.closeAllViews();
          Client.openView(Client.View.HOMEPAGE);
          JOptionPane.showMessageDialog(Client.homePageFrm, "Đối thủ không đồng ý thách đấu");
        }
        
        // Xử lý đồng ý thách đấu
        if(res.getState().equals("duel_accepted")){
          String[] splitter = res.getData().split(";");
          Pattern p = Pattern.compile(
            "game_id=(\\d+),is_start=(\\d+),ip=([\\.\\d]+),"
            + "id=(\\d+),username=([a-zA-Z0-9]+),avatar=([a-zA-Z0-9/\\.]+),game=(\\d+),"
            + "win=(\\d+),draw=(\\d+),loss=(\\d+),points=(\\d+),rank=(\\d+)"
          );
          Matcher m;
          int opponentID, roomID = 0, isStart = 0;
          String competitorIP = "0.0.0.0";
          User competitor = new User(0, "");
          for (String splitter1 : splitter) {
            m = p.matcher(splitter1);
            if(m.find()) {
              opponentID = Integer.parseInt(m.group(4));
              
              if(opponentID != Client.user.getID()) {
                roomID = Integer.parseInt(m.group(1));
                isStart = Integer.parseInt(m.group(2));
                competitorIP = m.group(3);
                competitor = new User(
                  opponentID, m.group(5), "", m.group(6),
                  Integer.parseInt(m.group(7)), Integer.parseInt(m.group(8)),
                  Integer.parseInt(m.group(9)), Integer.parseInt(m.group(10)),
                  Integer.parseInt(m.group(11)), Integer.parseInt(m.group(12))
                );
              }
            }
          }

          Client.closeAllViews();
          System.out.println("Đã vào phòng: " + roomID);
          Client.openView(Client.View.GAMECLIENT, competitor, roomID, isStart, competitorIP);
          Client.gameClientFrm.newgame();
        }


        /* ---------------------------------------------------------------------------------- */
        /*                                     GAME CARO                                      */
        /* ---------------------------------------------------------------------------------- */

        
        // Xử lý đánh một nước trong ván chơi
        if(res.getState().equals("caro")){
          Pattern p = Pattern.compile("x=(\\d+),y=(\\d+)");
          Matcher m = p.matcher(res.getData());
          m.find();
          Client.gameClientFrm.addCompetitorMove(m.group(1), m.group(2));
        }

        if(res.getState().equals("draw_request")){
          Client.gameClientFrm.showDrawRequest();
        }

        if(res.getState().equals("draw_refuse")){
          if(Client.gameNoticeFrm != null) Client.closeView(Client.View.GAMENOTICE);
          Client.gameClientFrm.displayDrawRefuse();
        }

        if(res.getState().equals("new_game")){
          System.out.println("New game");
          Thread.sleep(4000);
          Client.gameClientFrm.updateNumberOfGame();
          Client.closeView(Client.View.GAMENOTICE);
          Client.gameClientFrm.newgame();
        }

        if(res.getState().equals("draw")){
          System.out.println("Draw game");
          Client.closeView(Client.View.GAMENOTICE);
          Client.openView(Client.View.GAMENOTICE, "Ván chơi hòa", "Ván chơi mới dang được thiết lập");
          Client.gameClientFrm.displayDrawGame();
          Thread.sleep(4000);
          Client.gameClientFrm.updateNumberOfGame();
          Client.closeView(Client.View.GAMENOTICE);
          Client.gameClientFrm.newgame();
        }
        
        if(res.getState().equals("timeout")){
          Pattern p = Pattern.compile("game_id=(\\d+),opponent_id=(\\d+)");
          Matcher m = p.matcher(res.getData());
          m.find();
          int roomID = Integer.parseInt(m.group(1));
          int opponentID = Integer.parseInt(m.group(2));
          Client.gameClientFrm.increaseWinMatchToUser();

          Client.openView(Client.View.GAMENOTICE, "Bạn đã thắng do đối thủ quá thời gian", "Đang thiết lập ván chơi mới");
          Thread.sleep(4000);
          Client.closeView(Client.View.GAMENOTICE);
          Client.socketHandle.write(
            Client.socketHandle.requestify(
              "GAME_FINISH", 0, 
              "game_id=" + roomID + "&player_id=" + Client.user.getID() + "&opponent_id=" + opponentID + "&x=-1&y=-1&result=1&type=timeout", 
              ""
            )
          );
        }
      }
      
    } catch (UnknownHostException e) {
      e.printStackTrace();
    } catch (IOException e) {
      e.printStackTrace();
    } catch (InterruptedException ex) {
      ex.printStackTrace();
    }
  }

  public void write(String message) throws IOException{
    os.write(message);
    os.newLine();
    os.flush();
  }
  
  public String requestify(String command, int content_l, String params, String content) {
    return command + "#" + content_l + "#" + params + "#" + content;
  }

  public Socket getSocketOfClient() {
    return socketOfClient;
  }
}
