USE [GunzDB15]
GO
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER OFF
GO

CREATE OR ALTER FUNCTION [dbo].[inet_ntoa] (@IP BIGINT)
RETURNS VARCHAR(15)
AS
BEGIN
    DECLARE @NumIP BIGINT
    DECLARE @a INT, @b INT, @c INT, @d INT

    SET @NumIP = @IP

    SET @a = @NumIP / 16777216
    SET @NumIP = @NumIP % 16777216

    SET @b = @NumIP / 65536
    SET @NumIP = @NumIP % 65536

    SET @c = @NumIP / 256
    SET @NumIP = @NumIP % 256

    SET @d = @NumIP

    RETURN CAST(@a AS VARCHAR(3)) + '.' + CAST(@b AS VARCHAR(3)) + '.' +
           CAST(@c AS VARCHAR(3)) + '.' + CAST(@d AS VARCHAR(3))
END
GO
