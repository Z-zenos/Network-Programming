/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package model;

/**
 *
 * @author Admin
 */
public class User {
  private int ID;
  private String username;
  private String password;
  private String avatar;
  private int numberOfGame;
  private int numberOfWin;
  private int numberOfDraw;
  private int numberOfLoss;
  private boolean isOnline;
  private boolean isPlaying;
  private int points;
  private int rank;

  public User(int ID, String username, String password, String avatar, int numberOfGame, int numberOfWin, int numberOfDraw, int numberOfLoss, int points, int rank) {
    this.ID = ID;
    this.username = username;
    this.password = password;
    this.avatar = avatar;
    this.numberOfGame = numberOfGame;
    this.numberOfWin = numberOfWin;
    this.numberOfDraw = numberOfDraw;
    this.numberOfLoss = numberOfLoss;
    this.points = points;
    this.rank = rank;
  }

  public int getRank() {
    return rank;
  }

  public void setRank(int rank) {
    this.rank = rank;
  }

  public User(int ID, String username, String password, String avatar, int numberOfGame, int numberOfWin, int numberOfDraw, int numberOfLoss, boolean isOnline, boolean isPlaying) {
    this.ID = ID;
    this.username = username;
    this.password = password;
    this.avatar = avatar;
    this.numberOfGame = numberOfGame;
    this.numberOfWin = numberOfWin;
    this.numberOfDraw = numberOfDraw;
    this.numberOfLoss = numberOfLoss;
    this.isOnline = isOnline;
    this.isPlaying = isPlaying;
  }

  public User(int ID, String username, String password, String avatar, int numberOfGame, int numberOfWin, int numberOfDraw, int numberOfLoss) {
    this.ID = ID;
    this.username = username;
    this.password = password;
    this.avatar = avatar;
    this.numberOfGame = numberOfGame;
    this.numberOfWin = numberOfWin;
    this.numberOfDraw = numberOfDraw;
    this.numberOfLoss = numberOfLoss;
  }

  public User(int ID, String username) {
    this.ID = ID;
    this.username = username;
  }

  public User(int ID, String username, boolean isOnline, boolean isPlaying) {
    this.ID = ID;
    this.username = username;
    this.isOnline = isOnline;
    this.isPlaying = isPlaying;
  }

  public User(String username, String password) {
    this.username = username;
    this.password = password;
  }

  public User(String username, String password, String avatar) {
    this.username = username;
    this.password = password;
    this.avatar = avatar;
  }

  public int getID() {
    return ID;
  }

  public String getUsername() {
    return username;
  }

  public String getPassword() {
    return password;
  }

  public String getAvatar() {
    return avatar;
  }

  public int getNumberOfGame() {
    return numberOfGame;
  }

  public int getnumberOfWin() {
    return numberOfWin;
  }

  public boolean isIsOnline() {
    return isOnline;
  }

  public boolean isIsPlaying() {
    return isPlaying;
  }

  public void setUsername(String username) {
    this.username = username;
  }

  public void setPassword(String password) {
    this.password = password;
  }

  public void setAvatar(String avatar) {
    this.avatar = avatar;
  }

  public void setNumberOfGame(int numberOfGame) {
    this.numberOfGame = numberOfGame;
  }

  public void setnumberOfWin(int numberOfWin) {
    this.numberOfWin = numberOfWin;
  }

  public void setIsOnline(boolean isOnline) {
    this.isOnline = isOnline;
  }

  public void setIsPlaying(boolean isPlaying) {
    this.isPlaying = isPlaying;
  }

  public User(int ID, String username, int numberOfGame, int numberOfDraw, int numberOfLoss) {
    this.ID = ID;
    this.username = username;
    this.numberOfGame = numberOfGame;
    this.numberOfDraw = numberOfDraw;
    this.numberOfLoss = numberOfLoss;
  }

  public int getNumberOfDraw() {
    return numberOfDraw;
  }

  public void setNumberOfDraw(int numberOfDraw) {
    this.numberOfDraw = numberOfDraw;
  }

  public int getNumberOfLoss() {
    return numberOfLoss;
  }

  public void setNumberOfLoss(int numberOfLoss) {
    this.numberOfLoss = numberOfLoss;
  }

  public int getPoints() {
    return points;
  }

  public void setPoints(int points) {
    this.points = points;
  }
}
