use crate::models::{WorkshopItem, WorkshopSearchResult};

const WE_APP_ID: u32 = 431960;
const STEAM_API_BASE: &str = "https://api.steampowered.com";

#[derive(serde::Deserialize)]
struct SteamTag {
    tag: String,
}

#[derive(serde::Deserialize)]
struct SteamVoteData {
    score: Option<f64>,
    votes_up: Option<u64>,
    votes_down: Option<u64>,
}

#[derive(serde::Deserialize)]
struct SteamPublishedFile {
    publishedfileid: String,
    title: Option<String>,
    preview_url: Option<String>,
    short_description: Option<String>,
    tags: Option<Vec<SteamTag>>,
    subscriptions: Option<u64>,
    favorited: Option<u64>,
    vote_data: Option<SteamVoteData>,
    time_updated: Option<u64>,
    time_created: Option<u64>,
    file_size: Option<serde_json::Value>,
    views: Option<u64>,
}

#[derive(serde::Deserialize)]
struct QueryFilesResponse {
    response: QueryFilesInner,
}

#[derive(serde::Deserialize)]
struct QueryFilesInner {
    total: Option<serde_json::Value>,
    publishedfiledetails: Option<Vec<SteamPublishedFile>>,
}

fn parse_file(f: SteamPublishedFile) -> WorkshopItem {
    let tags: Vec<String> = f
        .tags
        .unwrap_or_default()
        .into_iter()
        .map(|t| t.tag)
        .collect();

    let content_type = tags
        .iter()
        .find(|t| {
            matches!(
                t.to_lowercase().as_str(),
                "video" | "scene" | "web" | "application"
            )
        })
        .cloned()
        .unwrap_or_else(|| "unknown".to_string());

    let file_size = match &f.file_size {
        Some(serde_json::Value::String(s)) => s.parse::<u64>().unwrap_or(0),
        Some(serde_json::Value::Number(n)) => n.as_u64().unwrap_or(0),
        _ => 0,
    };

    let (votes_up, votes_down, score) = f
        .vote_data
        .map(|v| {
            (
                v.votes_up.unwrap_or(0),
                v.votes_down.unwrap_or(0),
                v.score.unwrap_or(0.0),
            )
        })
        .unwrap_or((0, 0, 0.0));

    WorkshopItem {
        id: f.publishedfileid,
        title: f.title.unwrap_or_else(|| "Untitled".to_string()),
        preview_url: f.preview_url.unwrap_or_default(),
        description: f.short_description.unwrap_or_default(),
        tags,
        subscriptions: f.subscriptions.unwrap_or(0),
        favorited: f.favorited.unwrap_or(0),
        votes_up,
        votes_down,
        score,
        content_type,
        time_updated: f.time_updated.unwrap_or(0),
        time_created: f.time_created.unwrap_or(0),
        file_size,
        views: f.views.unwrap_or(0),
    }
}

#[tauri::command]
pub async fn search_workshop(
    api_key: String,
    query: String,
    page: u32,
    sort: String,
    type_filter: String,
) -> Result<WorkshopSearchResult, String> {
    if api_key.is_empty() {
        return Err("Steam API key not set. Add it in Settings.".to_string());
    }

    let query_type = match sort.as_str() {
        "new" => "1",
        "trend" => "9",
        "subscriptions" => "12",
        _ => "0",
    };

    let client = reqwest::Client::new();
    let mut req = client
        .get(format!("{}/IPublishedFileService/QueryFiles/v1/", STEAM_API_BASE))
        .query(&[
            ("key", api_key.as_str()),
            ("query_type", query_type),
            ("page", &page.to_string()),
            ("numperpage", "24"),
            ("appid", &WE_APP_ID.to_string()),
            ("return_vote_data", "1"),
            ("return_tags", "1"),
            ("return_previews", "1"),
            ("return_metadata", "1"),
            ("search_text", query.as_str()),
        ]);

    if !type_filter.is_empty() && type_filter != "all" {
        req = req.query(&[("required_tags", type_filter.as_str())]);
    }

    let resp = req
        .send()
        .await
        .map_err(|e| format!("Steam API request failed: {}", e))?;

    if !resp.status().is_success() {
        return Err(format!("Steam API error: {}", resp.status()));
    }

    let data: QueryFilesResponse = resp
        .json()
        .await
        .map_err(|e| format!("Failed to parse Steam API response: {}", e))?;

    let total = match &data.response.total {
        Some(serde_json::Value::String(s)) => s.parse::<u64>().unwrap_or(0),
        Some(serde_json::Value::Number(n)) => n.as_u64().unwrap_or(0),
        _ => 0,
    };

    let items = data
        .response
        .publishedfiledetails
        .unwrap_or_default()
        .into_iter()
        .map(parse_file)
        .filter(|item| !item.preview_url.is_empty())
        .collect();

    Ok(WorkshopSearchResult { items, total })
}

#[tauri::command]
pub async fn get_workshop_item(
    api_key: String,
    item_id: String,
) -> Result<WorkshopItem, String> {
    if api_key.is_empty() {
        return Err("Steam API key not set.".to_string());
    }

    let client = reqwest::Client::new();

    #[derive(serde::Deserialize)]
    struct DetailsResponse {
        response: DetailsInner,
    }
    #[derive(serde::Deserialize)]
    struct DetailsInner {
        publishedfiledetails: Option<Vec<SteamPublishedFile>>,
    }

    let resp = client
        .post(format!(
            "{}/ISteamRemoteStorage/GetPublishedFileDetails/v1/",
            STEAM_API_BASE
        ))
        .form(&[
            ("key", api_key.as_str()),
            ("itemcount", "1"),
            ("publishedfileids[0]", item_id.as_str()),
        ])
        .send()
        .await
        .map_err(|e| format!("Steam API request failed: {}", e))?;

    let data: DetailsResponse = resp
        .json()
        .await
        .map_err(|e| format!("Failed to parse response: {}", e))?;

    data.response
        .publishedfiledetails
        .and_then(|mut v| v.pop())
        .map(parse_file)
        .ok_or_else(|| "Item not found".to_string())
}
