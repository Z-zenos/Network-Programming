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
    List<User> friend = new ArrayList<>();
    for(int i = 1; i < message.length; i = i + 4){
      friend.add(new User(Integer.parseInt(message[i]), message[i + 1], message[i + 2].equals("1"), message[i + 3].equals("1")));
    }
    return friend;
  }
  
  public List<User> getListRank(String data){
    String[] splitter = data.split(";");
    List<User> player = new ArrayList<>();
    Pattern pattern = Pattern.compile("id=(\\d+)&username=([a-zA-Z0-9]+)&avatar=([a-zA-Z0-9/\\.]+)&win=(\\d+)&loss=(\\d+)&points=(-?\\d+)");
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
  
  // data: id=...&username=...&password=...&avatar=...&game=...&win=...&draw=...&loss=...&points=...&rank=...
  public User getUserFromString(String data){
    Pattern pattern = Pattern.compile(
      "id=(\\d+)&username=([a-zA-Z0-9]+)&password=([a-zA-Z0-9]+)&avatar=([a-zA-Z0-9/\\.]+)&"
          + "game=(\\d+)&win=(\\d+)&draw=(\\d+)&loss=(\\d+)&points=(\\d+)&rank=(\\d+)"
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
      Integer.parseInt(m.group(10))
    );
  }
  
  @Override
  public void run() {
    try {
      // Gửi yêu cầu kết nối tới Server đang lắng nghe
      socketOfClient = new Socket("127.0.0.1", 12121);
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
          System.out.println("Thông tin sai");
          Client.closeView(Client.View.GAMENOTICE);
          Pattern pattern = Pattern.compile("username=([a-zA-Z0-9]+)&password=([a-zA-Z0-9]+)");
          Matcher m = pattern.matcher(res.getData());
          m.find();
          Client.openView(Client.View.LOGIN, m.group(1), m.group(2));
          Client.loginFrm.showError("Username hoặc mật khẩu không chính xác");
        }
        
        // Tài khoản đã đăng nhập ở nơi khác
        if(res.getState().equals("login_duplicate")){
          System.out.println("Đã đăng nhập");
          Client.closeView(Client.View.GAMENOTICE);          
          Pattern pattern = Pattern.compile("username=([a-zA-Z0-9]+)&password=([a-zA-Z0-9]+)");
          Matcher m = pattern.matcher(res.getData());
          m.find();
          Client.openView(Client.View.LOGIN, m.group(1), m.group(2));
          Client.loginFrm.showError("Tài khoản đã đăng nhập ở nơi khác");
        }
//        
//        // Tài khoản đã bị banned
//        if(messageSplit[0].equals("banned-user")){
//          Client.closeView(Client.View.GAMENOTICE);
//          Client.openView(Client.View.LOGIN, messageSplit[1], messageSplit[2]);
//          Client.loginFrm.showError("Tài khoản đã bị ban");
//        }
//        
        // Xử lý register trùng tên
        if(res.getState().equals("username_duplicate")){
          Client.closeAllViews();
          Client.openView(Client.View.REGISTER);
          JOptionPane.showMessageDialog(Client.registerFrm, "Tên tài khoản đã được người khác sử dụng");
        }
 
        // Xử lý nhận thông tin, chat từ toàn server
        if(res.getState().equals("chat_global")){
          if(Client.homePageFrm != null){
            Client.homePageFrm.addMessage(res.getData());
          }
        }
//        
//        // Xử lý hiển thị thông tin đối thủ là bạn bè/không
//        if(messageSplit[0].equals("check-friend-response")){
//          if(Client.competitorInfoFrm != null){
//            Client.competitorInfoFrm.checkFriend((messageSplit[1].equals("1")));
//          }
//        }
//        
//        // Xử lý kết quả tìm phòng từ server
//        if(messageSplit[0].equals("room-fully")){
//          Client.closeAllViews();
//          Client.openView(Client.View.HOMEPAGE);
//          JOptionPane.showMessageDialog(Client.homePageFrm, "Phòng chơi đã đủ 2 người chơi");
//        }
//        
//        // Xử lý không tìm thấy phòng trong chức năng vào phòng
//        if(messageSplit[0].equals("room-not-found")){
//          Client.closeAllViews();
//          Client.openView(Client.View.HOMEPAGE);
//          JOptionPane.showMessageDialog(Client.homePageFrm, "Không tìm thấy phòng");
//        }
//        
        // Xử lý phòng có mật khẩu sai
        if(res.getState().equals("game_password_incorrect")){
          Client.closeAllViews();
          Client.openView(Client.View.HOMEPAGE);
          JOptionPane.showMessageDialog(Client.homePageFrm, "Mật khẩu phòng sai");
        }
        
        // Xử lý xem rank
        if(res.getState().equals("rank")){
          if(Client.rankFrm != null){
            Client.rankFrm.setDataToTable(getListRank(res.getData()));
          }
        }

        // Xử lý lấy danh sách phòng
        if(res.getState().equals("game_list")){
          Vector<String> rooms = new Vector<>();
          Vector<String> passwords = new Vector<>();
          String[] splitter = res.getData().split(";");
          Pattern p = Pattern.compile("game_id=(\\d+)&password=([a-zA-Z0-9]+)&views=(\\d+)&num_move=(\\d+)&player1_id=(\\d+)&player2_id=(\\d+)");
          Matcher m;
          for(int i = 0; i < splitter.length; i++){
            m = p.matcher(splitter[i]);
            if(m.find()) {
              rooms.add("Phòng " + m.group(1));
              passwords.add(m.group(2));
              System.out.println("Room: " + m.group(1) + " - Password: " + m.group(2));
            }
            
          }
          Client.roomListFrm.updateRoomList(rooms, passwords);
        }
//        
//        // Xử lý danh sách bạn bè 
//        if(messageSplit[0].equals("return-friend-list")){
//          if(Client.friendListFrm != null){
//            Client.friendListFrm.updateFriendList(getListUser(messageSplit));
//          }
//        }
//        
//        // Xử lý vào phòng 
//        if(messageSplit[0].equals("go-to-room")){
//          System.out.println("Vào phòng");
//          int roomID = Integer.parseInt(messageSplit[1]);
//          String competitorIP = messageSplit[2];
//          int isStart = Integer.parseInt(messageSplit[3]);
//
//          User competitor = getUserFromString(4, messageSplit);
//          if(Client.findRoomFrm != null){
//            Client.findRoomFrm.showFindedRoom();
//            try {
//              Thread.sleep(3000);
//            } catch (InterruptedException ex) {
//              JOptionPane.showMessageDialog(Client.findRoomFrm, "Lỗi khi sleep thread");
//            }
//          } 
//          else if(Client.waitingRoomFrm != null){
//            Client.waitingRoomFrm.showFindedCompetitor();
//            try {
//              Thread.sleep(3000);
//            } catch (InterruptedException ex) {
//              JOptionPane.showMessageDialog(Client.waitingRoomFrm, "Lỗi khi sleep thread");
//            }
//          }
//          Client.closeAllViews();
//          System.out.println("Đã vào phòng: "+roomID);
//          
//          Client.openView(Client.View.GAMECLIENT, competitor, roomID, isStart, competitorIP);
//          Client.gameClientFrm.newgame();
//        }
//        
        // Tạo phòng và server trả về tên phòng
        if(res.getState().equals("game_created")){
          Client.closeAllViews();
          Client.openView(Client.View.WAITINGROOM);
          String[] data = res.parseData("game_id=(\\d+)&password=([a-zA-Z0-9]+)");
          Client.waitingRoomFrm.setRoomName(data[0]);
//          if(messageSplit.length == 3)
          Client.waitingRoomFrm.setRoomPassword("Mật khẩu phòng: " + data[1]);
        }
//        
//        // Xử lý yêu cầu kết bạn tới
//        if(messageSplit[0].equals("make-friend-request")){
//          int ID = Integer.parseInt(messageSplit[1]);
//          String nickname = messageSplit[2];
//          Client.openView(Client.View.FRIENDREQUEST, ID, nickname);
//        }
//        
//        // Xử lý khi nhận được yêu cầu thách đấu
//        if(messageSplit[0].equals("duel-notice")){
//          int res = JOptionPane.showConfirmDialog(
//            Client.getVisibleJFrame(), 
//            "Bạn nhận được lời thách đấu của " + messageSplit[2]+ " (ID=" + messageSplit[1]+ ")", 
//            "Xác nhận thách đấu", 
//            JOptionPane.YES_NO_OPTION
//          );
//          if(res == JOptionPane.YES_OPTION){
//            Client.socketHandle.write("agree-duel," + messageSplit[1]);
//          }
//          else{
//            Client.socketHandle.write("disagree-duel," + messageSplit[1]);
//          }
//        }
//        
//        // Xử lý không đồng ý thách đấu
//        if(messageSplit[0].equals("disagree-duel")){
//          Client.closeAllViews();
//          Client.openView(Client.View.HOMEPAGE);
//          JOptionPane.showMessageDialog(Client.homePageFrm, "Đối thủ không đồng ý thách đấu");
//        }
//        
//        // Xử lý đánh một nước trong ván chơi
//        if(messageSplit[0].equals("caro")){
//          Client.gameClientFrm.addCompetitorMove(messageSplit[1], messageSplit[2]);
//        }
//        if(messageSplit[0].equals("chat")){
//          Client.gameClientFrm.addMessage(messageSplit[1]);
//        }
//        if(messageSplit[0].equals("draw-request")){
//          Client.gameClientFrm.showDrawRequest();
//        }
//
//        if(messageSplit[0].equals("draw-refuse")){
//          if(Client.gameNoticeFrm != null) Client.closeView(Client.View.GAMENOTICE);
//          Client.gameClientFrm.displayDrawRefuse();
//        }
//
//        if(messageSplit[0].equals("new-game")){
//          System.out.println("New game");
//          Thread.sleep(4000);
//          Client.gameClientFrm.updateNumberOfGame();
//          Client.closeView(Client.View.GAMENOTICE);
//          Client.gameClientFrm.newgame();
//        }
//        
//        if(messageSplit[0].equals("draw-game")){
//          System.out.println("Draw game");
//          Client.closeView(Client.View.GAMENOTICE);
//          Client.openView(Client.View.GAMENOTICE, "Ván chơi hòa", "Ván chơi mới dang được thiết lập");
//          Client.gameClientFrm.displayDrawGame();
//          Thread.sleep(4000);
//          Client.gameClientFrm.updateNumberOfGame();
//          Client.closeView(Client.View.GAMENOTICE);
//          Client.gameClientFrm.newgame();
//        }
//        
//        if(messageSplit[0].equals("competitor-time-out")){
//          Client.gameClientFrm.increaseWinMatchToUser();
//          Client.openView(Client.View.GAMENOTICE, "Bạn đã thắng do đối thủ quá thới gian","Đang thiết lập ván chơi mới");
//          Thread.sleep(4000);
//          Client.closeView(Client.View.GAMENOTICE);
//          Client.gameClientFrm.updateNumberOfGame();
//          Client.gameClientFrm.newgame();
//        }
//        
//        // Xử lý rời phòng
//        if(messageSplit[0].equals("left-room")){
//          Client.gameClientFrm.stopTimer();
//          Client.closeAllViews();
//          Client.openView(Client.View.GAMENOTICE, "Đối thủ đã thoát khỏi phòng", "Đang trở về trang chủ");
//          Thread.sleep(3000);       
//          Client.closeAllViews();
//          Client.openView(Client.View.HOMEPAGE);
//        }
//        
//        //Xử lý bị banned
//        if(messageSplit[0].equals("banned-notice")){
//          Client.socketHandle.write("offline," + Client.user.getID());
//          Client.closeAllViews();
//          Client.openView(Client.View.LOGIN);
//          JOptionPane.showMessageDialog(Client.loginFrm, messageSplit[1], "Bạn đã bị BAN", JOptionPane.WARNING_MESSAGE);
//        }
//        
//        //Xử lý cảnh cáo
//        if(messageSplit[0].equals("warning-notice")){
//          JOptionPane.showMessageDialog(null, messageSplit[1] , "Bạn nhận được một cảnh cáo", JOptionPane.WARNING_MESSAGE);
//        }
      }
    } catch (UnknownHostException e) {
      e.printStackTrace();
    } catch (IOException e) {
      e.printStackTrace();
    } 
//    catch (InterruptedException ex) {
//      ex.printStackTrace();
//    }
  }

  public void write(String message) throws IOException{
    os.write(message);
    os.newLine();
    os.flush();
  }

  public Socket getSocketOfClient() {
    return socketOfClient;
  }
}
