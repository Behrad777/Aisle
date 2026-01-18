import { DynamoDBClient } from "@aws-sdk/client-dynamodb";
import { DynamoDBDocumentClient, ScanCommand } from "@aws-sdk/lib-dynamodb";

const client = new DynamoDBClient({ region: process.env.AWS_REGION });
export const docClient = DynamoDBDocumentClient.from(client);

export async function scanAll() {
  const res = await docClient.send(
    new ScanCommand({
      TableName: "AisleCartTelemetry",
    })
  );
  return res.Items ?? [];
}