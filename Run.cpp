#include "DxLib.h"
#include <stdlib.h>
#include <math.h>

// バイクの画像を管理する定数と配列
const int BIKE_MAX = 2;
int imgBike[BIKE_MAX];

// バイクの表示位置と判定四角形の更新
void DrawBikeAndHitBox(int x, int y, int type, int* rL, int* rR, int* rT, int* rB, int* fL, int* fR, int* fT, int* fB)
{
	*rL = x + 5;       // 後輪の左端
	*rR = x + 95;       // 後輪の右端
	*rT = y + 125;      // タイヤのゴム部分
	*rB = y + 155;      // タイヤの接地している一番底辺

	*fL = x + 190;      // 前輪の左端
	*fR = x + 290;      // 前輪の右端
	*fT = y + 125;      // タイヤのゴム部分
	*fB = y + 155;      // タイヤの接地している一番底辺
}

// 2つの円が当たっているか計算する関数
bool CheckCircleHit(int c1x, int c1y, int c1r, int c2x, int c2y, int c2r)
{
	int dx = c1x - c2x;
	int dy = c1y - c2y;
	int distanceSquare = dx * dx + dy * dy;
	int radiusSum = c1r + c2r;
	return distanceSquare < (radiusSum * radiusSum);
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
	const int WIDTH = 1014, HEIGHT = 768;

	SetWindowText("バイクゲーム");
	SetGraphMode(WIDTH, HEIGHT, 32);
	ChangeWindowMode(TRUE);
	if (DxLib_Init() == -1) return -1;
	SetBackgroundColor(0, 0, 0);
	SetDrawScreen(DX_SCREEN_BACK);

	int bgX = 0;
	int imgBG = LoadGraph("Image/BG.jpg");

	// 2枚の画像ファイルを配列に読み込む
	imgBike[0] = LoadGraph("Image/Redbike0.png");
	imgBike[1] = LoadGraph("Image/Redbike1.png");

	// 岩の画像を読み込む
	int imgObstacle = LoadGraph("Image/Rock.png");
	const int OBSTACLE_WIDTH = 130, OBSTACLE_HEIGHT = 130;
	// 無敵アイテムの画像を読み込む
	int imgItem = LoadGraph("Image/Item1.png");
	const int ITEM_WIDTH = 50, ITEM_HEIGHT = 50;
	const int GHOST_MAX = 5;
	int ghostX[5] = { 0 };
	int ghostY[5] = { 0 };
	int ghostFrame[5] = { 0 };
	int ghostCount = 0;

	// スコアアップアイテムの画像を読み込む
	int imgScoreUP = LoadGraph("Image/ScoreUP.png");
	const int SCOREUP_WIDTH = 50, SCOREUP_HEIGHT = 50;
	// BGMを読み込む
	int soundBGM = LoadSoundMem("Sound/BGM.mp3");
	PlaySoundMem(soundBGM, DX_PLAYTYPE_LOOP, TRUE);
	// バイクが壊れた時のSEを読み込む
	int soundHit = LoadSoundMem("Sound/BikeSE.mp3");
	// 無敵アイテムを取った時のSEを読み込む
	int soundItem = LoadSoundMem("Sound/Item1SE.mp3");
	// スコアアイテムを取った時のSEを読み込む
	int soundScoreUP = LoadSoundMem("Sound/ScoreUPSE.mp3");
	// 岩を破壊した時のSEを読み込む
	int soundDestroy = LoadSoundMem("Sound/DestroySE.mp3");
	// 無敵中のSEを読み込む
	int soundInvincible = LoadSoundMem("Sound/InvincibleSE.mp3");

	const int FLOOR_Y = 472;

	int playerX = 100;
	int playerY = FLOOR_Y;
	int playerVY = 0;

	int obstacleX = WIDTH;
	int obstacleY = FLOOR_Y + 20;
	int obstacleSpeed = 15;

	int itemX = WIDTH + 1500;
	int itemY = FLOOR_Y - 100;
	int itemSpeed = 15;
	int isItemExit = 1;

	int scoreUpX = WIDTH + 2000;
	int scoreUpY = FLOOR_Y - 80;
	int scoreUpSpeed = 15;
	int isScoreUpExit = 1;

	// 無敵タイマーとゲームオーバーフラグ
	int invincibleTimer = 0;
	// ゲームオーバーフラグ
	int isGameOver = 0;
	// スコア
	int score = 0;
	// ハイスコア
	int hiscore = 0;

	int gameState = 0;

	int comboCount = 0;

	// アニメーション制御用の変数
	int animeTimer = 0;
	int animeFrame = 0;

	int rearLeft = 0, rearRight = 0, rearTop = 0, rearBottom = 0;
	int frontLeft = 0, frontRight = 0, frontTop = 0, frontBottom = 0;

	// メインループ
	while (ProcessMessage() == 0 && CheckHitKey(KEY_INPUT_ESCAPE) == 0)
	{
		// 画面をクリア
		ClearDrawScreen();

		// タイトル画面の処理
		if (gameState == 0)
		{
			bgX = bgX - 2;
			if (bgX <= -WIDTH) bgX = bgX + WIDTH;

			if (CheckHitKey(KEY_INPUT_SPACE) == 1)
			{
				playerX = 100;
				playerY = FLOOR_Y;
				playerVY = 0;
				obstacleX = WIDTH;
				obstacleY = FLOOR_Y + 20;
				itemX = WIDTH + rand() % 2000;
				itemY = FLOOR_Y - 100;
				isItemExit = 1;
				scoreUpX = WIDTH + rand() % 2000 + 1000;
				scoreUpY = FLOOR_Y - 80;
				isScoreUpExit = 1;

				invincibleTimer = 0;
				isGameOver = 0;
				score = 0;
				comboCount = 0;
				animeTimer = 0;
				animeFrame = 0;

				StopSoundMem(soundInvincible);
				PlaySoundMem(soundBGM, DX_PLAYTYPE_LOOP, TRUE);
				gameState = 1;
			}
		}
		// ゲームプレイ中の処理
		else if (gameState == 1)
		{
			if (isGameOver == 0)
			{
				// 無敵タイマーの処理
				if (invincibleTimer > 0)
				{
					invincibleTimer--;
					if (invincibleTimer == 0)
					{
						StopSoundMem(soundInvincible);
						PlaySoundMem(soundBGM, DX_PLAYTYPE_LOOP, TRUE);
					}
				}

				// ゲームの更新処理 
				bgX = bgX - 2;
				if (bgX <= -WIDTH) bgX = bgX + WIDTH;

				// スペースキーでジャンプ
				if (CheckHitKey(KEY_INPUT_SPACE) == 1 && playerY == FLOOR_Y)
				{
					playerVY = -20;
				}

				// 重力の処理
				playerVY += 1;
				playerY += playerVY;

				if (playerY > FLOOR_Y)
				{
					playerY = FLOOR_Y;
					playerVY = 0;

					// 2枚が交互にパラパラ動くタイマー計算
					animeTimer++;
					if (animeTimer % 6 == 0) // ここの「6」の数値でパタパタ動く速さを変える
					{
						animeFrame = (animeFrame + 1) % BIKE_MAX; // 0→1→0→1とループ
					}
				}
				else
				{
					animeFrame = 0;
				}

				// 障害物の移動
				obstacleX -= obstacleSpeed;
				if (obstacleX < -OBSTACLE_WIDTH)
				{
					obstacleX = WIDTH;
					obstacleY = FLOOR_Y + 20;
					score += 10;
					if (score > hiscore) hiscore = score;
				}

				// 無敵アイテムの移動
				itemX -= itemSpeed;
				if (itemX < -ITEM_WIDTH)
				{
					itemX = WIDTH + rand() % 2000 + rand() % 2500;
					itemY = FLOOR_Y - 100;
					isItemExit = 1;
				}

				// スコアアップアイテムの移動
				scoreUpX -= scoreUpSpeed;
				if (scoreUpX < -SCOREUP_WIDTH)
				{
					comboCount = 0;

					scoreUpX = WIDTH + rand() % 2000 + rand() % 2500;
					scoreUpY = FLOOR_Y - (60 + rand() % 60);
					isScoreUpExit = 1;

					// スコアアップアイテムが障害物と重ならないようにする
					if (abs(scoreUpX - itemX) < 400)
					{
						scoreUpX += 500;
					}
				}

				// 毎フレームのプレイヤーの位置を残像配列に記録
				ghostX[ghostCount] = playerX;
				ghostY[ghostCount] = playerY;
				ghostFrame[ghostCount] = animeFrame;
				ghostCount = (ghostCount + 1) % GHOST_MAX;

				DrawBikeAndHitBox(playerX, playerY, animeFrame,
					&rearLeft, &rearRight, &rearTop, &rearBottom,
					&frontLeft, &frontRight, &frontTop, &frontBottom);

				// 障害物の中心座標と半径を計算
				int oCenter_X = obstacleX + 80; int oCenter_Y = obstacleY + 95; int oCenter_R = 45;
				int oLeft_X = obstacleX + 35;   int oLeft_Y = obstacleY + 130;  int oLeft_R = 25;
				int oRight_X = obstacleX + 120; int oRight_Y = obstacleY + 130; int oRight_R = 25;

				int iCenter_X = itemX + (ITEM_WIDTH / 2);
				int iCenter_Y = itemY + (ITEM_HEIGHT / 2);
				int iCenter_R = 30;

				int rearCenterX = (rearLeft + rearRight) / 2;
				int rearCenterY = (rearTop + rearBottom) / 2;
				int rearRadius = (rearRight - rearLeft) / 2;

				int frontCenterX = (frontLeft + frontRight) / 2;
				int frontCenterY = (frontTop + frontBottom) / 2;
				int frontRadius = (frontRight - frontLeft) / 2;
				// バイクの当たり判定を円形に変更
				if (isItemExit == 1)
				{
					if (CheckCircleHit(rearCenterX, rearCenterY, rearRadius, iCenter_X, iCenter_Y, iCenter_R) ||
						CheckCircleHit(frontCenterX, frontCenterY, frontRadius, iCenter_X, iCenter_Y, iCenter_R))
					{
						isItemExit = 0;
						invincibleTimer = 180;
						score += 30;
						if (score > hiscore) hiscore = score;

						PlaySoundMem(soundItem, DX_PLAYTYPE_BACK, TRUE);
						StopSoundMem(soundBGM);
						PlaySoundMem(soundInvincible, DX_PLAYTYPE_BACK, TRUE);
					}
				}


				if (isScoreUpExit == 1)
				{
					// スコアアップアイテムの中心座標と半径を計算
					int suCenter_X = scoreUpX + (SCOREUP_WIDTH / 2);
					int suCenter_Y = scoreUpY + (SCOREUP_HEIGHT / 2);
					int suCenter_R = 25;

					// スコアアップアイテムの当たり判定を円形に変更
					if (CheckCircleHit(rearCenterX, rearCenterY, rearRadius, suCenter_X, suCenter_Y, suCenter_R) ||
						CheckCircleHit(frontCenterX, frontCenterY, frontRadius, suCenter_X, suCenter_Y, suCenter_R))
					{
						isScoreUpExit = 0;
						comboCount++;
						score += 100 * comboCount;
						if (score > hiscore) hiscore = score;


						PlaySoundMem(soundScoreUP, DX_PLAYTYPE_BACK, TRUE);
					}
				}


				bool isRearHit = CheckCircleHit(rearCenterX, rearCenterY, rearRadius, oCenter_X, oCenter_Y, oCenter_R) ||
					CheckCircleHit(rearCenterX, rearCenterY, rearRadius, oLeft_X, oLeft_Y, oLeft_R) ||
					CheckCircleHit(rearCenterX, rearCenterY, rearRadius, oRight_X, oRight_Y, oRight_R);

				bool isFrontHit = CheckCircleHit(frontCenterX, frontCenterY, frontRadius, oCenter_X, oCenter_Y, oCenter_R) ||
					CheckCircleHit(frontCenterX, frontCenterY, frontRadius, oLeft_X, oLeft_Y, oLeft_R) ||
					CheckCircleHit(frontCenterX, frontCenterY, frontRadius, oRight_X, oRight_Y, oRight_R);

				if (isRearHit || isFrontHit)
				{
					if (invincibleTimer > 0)
					{
						obstacleX = WIDTH;
						obstacleY = FLOOR_Y + 20;
						score += 20;
						if (score > hiscore) hiscore = score;
						PlaySoundMem(soundDestroy, DX_PLAYTYPE_BACK, TRUE);
					}
					else
					{
						isGameOver = 1;
						StopSoundMem(soundBGM);
						StopSoundMem(soundInvincible);
						PlaySoundMem(soundHit, DX_PLAYTYPE_BACK, TRUE);
					}
				}
			}
			else
			{

				if (CheckHitKey(KEY_INPUT_R) == 1)
				{
					playerX = 100;
					playerY = FLOOR_Y;
					playerVY = 0;
					obstacleX = WIDTH;
					obstacleY = FLOOR_Y + 20;
					itemX = WIDTH + rand() % 2000;
					itemY = FLOOR_Y - 100;
					isItemExit = 1;
					scoreUpX = WIDTH + rand() % 2000 + 1000;
					scoreUpY = FLOOR_Y - 80;
					isScoreUpExit = 1;

					invincibleTimer = 0;
					isGameOver = 0;
					score = 0;
					comboCount = 0;
					animeTimer = 0;
					animeFrame = 0;

					StopSoundMem(soundInvincible);
					PlaySoundMem(soundBGM, DX_PLAYTYPE_LOOP, TRUE);

					DrawBikeAndHitBox(playerX, playerY, animeFrame, &rearLeft, &rearRight, &rearTop, &rearBottom, &frontLeft, &frontRight, &frontTop, &frontBottom);
				}

				if (CheckHitKey(KEY_INPUT_T) == 1)
				{
					gameState = 0;
				}
			}
		}

		// 描画セクション
		DrawGraph(bgX, 0, imgBG, FALSE);
		DrawGraph(bgX + WIDTH, 0, imgBG, FALSE);

		if (gameState == 0)
		{
			DrawString(WIDTH / 2 - 70, HEIGHT / 2 - 20, "BIKE GAME", GetColor(255, 255, 255));
			DrawString(WIDTH / 2 - 140, HEIGHT / 2 + 10, "Press Space Key to Start", GetColor(255, 255, 0));
		}
		else if (gameState == 1)
		{

			DrawExtendGraph(obstacleX, obstacleY + 20, obstacleX + OBSTACLE_WIDTH, obstacleY + OBSTACLE_HEIGHT + 20, imgObstacle, TRUE);

			if (isItemExit == 1)
			{
				DrawExtendGraph(itemX, itemY, itemX + ITEM_WIDTH, itemY + ITEM_HEIGHT, imgItem, TRUE);
			}


			if (isScoreUpExit == 1)
			{
				DrawExtendGraph(scoreUpX, scoreUpY, scoreUpX + SCOREUP_WIDTH, scoreUpY + SCOREUP_HEIGHT, imgScoreUP, TRUE);
			}

			// バイクの描画
			float angle = 0.0f; // 通常時（地面）は 0度（水平）
			if (playerY < FLOOR_Y)
			{
				angle = playerVY * 0.015f; // ここの 0.015f の数値を大きくするとより激しく傾く

				// 傾きが大きくなりすぎないように制限（最大約25度まで）
				if (angle < -0.45f) angle = -0.45f;
				if (angle > 0.45f) angle = 0.45f;
			}

			// バイク画像1枚あたりのサイズ
			const int BIKE_HALF_W = 141.5;
			const int BIKE_HALF_H = 85;

			// 本体と無敵残像の描画
			if (invincibleTimer > 0)
			{
				// 無敵状態の描画

				// 残像の描画
				for (int i = 0; i < GHOST_MAX; i++)
				{
					int idx = (ghostCount + i) % GHOST_MAX;
					int alpha = 50 + (i * 40);

					SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);

					int offset_x = ghostX[idx] - (GHOST_MAX - i) * 6;

					DrawRotaGraph(offset_x + BIKE_HALF_W, ghostY[idx] + BIKE_HALF_H, 1.0, angle, imgBike[ghostFrame[idx]], TRUE);
				}

				SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

				// 本体を点滅描画
				if ((invincibleTimer / 4) % 2 == 0)
				{
					// DrawRotaGraph に変更
					DrawRotaGraph(playerX + BIKE_HALF_W, playerY + BIKE_HALF_H, 1.0, angle, imgBike[animeFrame], TRUE);
				}
			}
			else
			{
				// 通常状態の描画
				DrawRotaGraph(playerX + BIKE_HALF_W, playerY + BIKE_HALF_H, 1.0, angle, imgBike[animeFrame], TRUE);
			}

			// スコアとハイスコアの描画
			DrawFormatString(20, 20, GetColor(255, 255, 255), "SCORE: %d", score);
			DrawFormatString(20, 50, GetColor(255, 255, 0), "HISCORE: %d", hiscore);

			if (comboCount >= 2)
			{
				SetFontSize(40);
				DrawFormatString(WIDTH / 2 - 80, 100, GetColor(255, 200, 0), "%d COMBO!", comboCount); SetFontSize(16);
			}
			if (invincibleTimer > 0)
			{
				DrawBox( 20, 80, 20 + invincibleTimer, 95, GetColor(255, 128, 0), TRUE); 
				DrawString( 25, 80, "INVINCIBLE!", GetColor(255, 255, 255));
			}
			if (isGameOver == 1)
			{
				DrawString(WIDTH / 2 - 100, HEIGHT / 2 - 20, "GAME OVER", GetColor(255, 0, 0));
				DrawString(WIDTH / 2 - 140, HEIGHT / 2 + 10, "Press R Key to Restart / T Key to Title", GetColor(255, 255, 255));
			}
		}


		ScreenFlip();
		WaitTimer(16);
	}

	DxLib_End();
	return 0;
}