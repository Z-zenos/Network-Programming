/*
 * Click nbfs://nbhost/SystemFileSystem/Templates/Licenses/license-default.txt to change this license
 * Click nbfs://nbhost/SystemFileSystem/Templates/Classes/Class.java to edit this template
 */
package model;

import java.util.regex.MatchResult;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

/**
 *
 * @author zenos
 */
public class Response {
  private int code;
  private String data;
  private String state;
  private String message;

  public Response(String response) {
    Pattern pattern = Pattern.compile(
      "code: (\\d+),state: ([a-zA-Z0-9_]+),data: (.+),message: (.+)"
    );
    Matcher m = pattern.matcher(response);
    m.find();
    this.code = Integer.parseInt(m.group(1)); // code: 200
    this.state = m.group(2); // state: wrong-user
    this.data = m.group(3);  // data: username=tuan12&password=absa12xsh
    this.message = m.group(4); // message: Successfully\n
  }

  public int getCode() {
    return code;
  }

  public String getData() {
    return data;
  }

  public String getState() {
    return state;
  }

  public String getMessage() {
    return message;
  }

  public String[] parseData(String format) {
    return Pattern.compile(format)
      .matcher(this.getData())
      .results()
      .map(MatchResult::group)
      .toArray(String[]::new);
  }
}
