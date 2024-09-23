import { WebSocketServer } from "ws";

const wss = new WebSocketServer({ port: 9000 });

wss.on("connection", (ws) => {
  const ip = req.socket.remoteAddress;
  console.log(`Handshake successful with client with IP ${ip}`);

  ws.on("error", console.error);

  ws.on("open", () => {
    ws.send("WebSocket server opened");
  });

  ws.on("message", (msg) => {
    console.log("received: %s", msg);
  });

  ws.send("ok");
});
