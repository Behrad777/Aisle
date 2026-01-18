// src/pages/api/thing-attributes.ts
import type { NextApiRequest, NextApiResponse } from "next";
import { IoTClient, DescribeThingCommand } from "@aws-sdk/client-iot";

const iot = new IoTClient({ region: "ca-central-1" });

export default async function handler(
  req: NextApiRequest,
  res: NextApiResponse
) {
  // grab thingName from query
  const raw = req.query.thingName;
  const thingName = Array.isArray(raw) ? raw[0] : raw;

  if (typeof thingName !== "string") {
    return res.status(400).json({ error: "Missing query param `thingName`" });
  }

  try {
    const { attributes } = await iot.send(
      new DescribeThingCommand({ thingName })
    );
    res.status(200).json({ attributes });
  } catch (e: any) {
    res.status(500).json({ error: e.message });
  }
}
