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
  private String data;
  private String state;

  public Response(String response) {
    Pattern pattern = Pattern.compile("RESPONSE#0#0#(.+)");
    Matcher m = pattern.matcher(response);
    m.find();
    String content = m.group(1);
    String[] splitter = content.split(",");
    this.state = splitter[0].substring(splitter[0].indexOf("=") + 1);
    this.data = content.contains(",") ? content.substring(content.indexOf(",") + 1) : "";
  }

  public String getData() {
    return data;
  }
  
  public String getState() {
    return this.state;
  }

  public String[] parseData(String format) {
    return Pattern.compile(format)
      .matcher(this.getData())
      .results()
      .map(MatchResult::group)
      .toArray(String[]::new);
  }
}
