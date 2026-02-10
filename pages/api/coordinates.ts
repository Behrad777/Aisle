import type { NextApiRequest, NextApiResponse } from "next";
import { scanAll } from "./dynamo"; // adjust path if needed

export default async function handler(
  req: NextApiRequest,
  res: NextApiResponse
) {
  try {
    const items = await scanAll();
    res.status(200).json(items ?? []);
  } catch (err) {
    res.status(500).json({ error: "Failed to fetch coordinates" });
  }
}